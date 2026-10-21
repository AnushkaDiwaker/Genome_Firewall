#include "ScoringEngine.h"
#include <iostream>

// ---------------------------------------------------------------------------
// Thread entry point called by CreateThread / pthread_create
// ---------------------------------------------------------------------------

#ifdef _WIN32
DWORD WINAPI ScoringEngine::threadEntry(LPVOID param) {
    ChunkTask* task = static_cast<ChunkTask*>(param);
    task->engine->processSequenceChunk(task->sequence, task->kmerLength);
    delete task;
    return 0;
}
#else
void* ScoringEngine::threadEntry(void* param) {
    ChunkTask* task = static_cast<ChunkTask*>(param);
    task->engine->processSequenceChunk(task->sequence, task->kmerLength);
    delete task;
    return NULL;
}
#endif

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ScoringEngine::ScoringEngine(const AMRTrie& trie, const AMRDatabase& db,
                             bool enableFuzzy, double confidenceThreshold)
    : amrTrie(trie),
      amrDb(db),
      fuzzyMatchEnabled(enableFuzzy),
      predictor(trie, confidenceThreshold) {
    size_t numRecords = amrDb.getRecords().size();
    if (numRecords == 0) numRecords = 10;
    featureVectorAtomics = std::vector<std::atomic<int>>(numRecords);
    for (auto& atom : featureVectorAtomics) {
        atom.store(0, std::memory_order_relaxed);
    }
}

// ---------------------------------------------------------------------------
// High-Throughput Single-Pass Chunk Processing
// ---------------------------------------------------------------------------

void ScoringEngine::processSequenceChunk(const std::string& sequence, size_t kmerLength) {
    if (sequence.length() < kmerLength) return;

    int localExact   = 0;
    int localMutated = 0;

    const auto& records = amrDb.getRecords();
    const size_t numRecords = records.size();

    std::unordered_map<std::string, int> localDirectHits;
    std::unordered_map<std::string, int> localMutatedHits;
    std::unordered_map<std::string, bool> localTargetIntegrity;

    const size_t maxI = sequence.length() - kmerLength;
    for (size_t i = 0; i <= maxI; ++i) {
        std::string kmer = sequence.substr(i, kmerLength);

        int recordIdx = -1;
        // Fast-path exact match with Bloom filter pre-check
        if (amrTrie.searchExact(kmer, &recordIdx)) {
            localExact++;
            if (recordIdx >= 0 && static_cast<size_t>(recordIdx) < numRecords) {
                featureVectorAtomics[recordIdx].store(1, std::memory_order_relaxed);
                const auto& rec = records[recordIdx];
                if (rec.recordType == "TargetGene") {
                    localTargetIntegrity[rec.targetDrug] = true;
                } else {
                    localDirectHits[rec.targetDrug]++;
                }
            }
        } else if (fuzzyMatchEnabled && amrTrie.searchWithMismatch(kmer, 1, &recordIdx)) {
            localMutated++;
            if (recordIdx >= 0 && static_cast<size_t>(recordIdx) < numRecords) {
                featureVectorAtomics[recordIdx].store(1, std::memory_order_relaxed);
                const auto& rec = records[recordIdx];
                if (rec.recordType != "TargetGene") {
                    localMutatedHits[rec.targetDrug]++;
                }
            }
        }
    }

    if (localExact   > 0) exactMatches.fetch_add(localExact, std::memory_order_relaxed);
    if (localMutated > 0) mutatedMatches.fetch_add(localMutated, std::memory_order_relaxed);

    int localScore = localExact + localMutated;
    if (localScore > 0) {
        resistanceScore.fetch_add(localScore, std::memory_order_relaxed);
    }

    // Merge thread-local hits into predictor under mutex
    {
        WinLockGuard lock(tasksMutex);
        predictor.mergeHits(localDirectHits, localMutatedHits, localTargetIntegrity, localExact, localMutated);
    }
}

// ---------------------------------------------------------------------------
// Async Chunk Dispatcher (spawns native OS thread per chunk)
// ---------------------------------------------------------------------------

void ScoringEngine::processSequenceChunkAsync(const std::string& sequence, size_t kmerLength) {
    ChunkTask* task = new ChunkTask{this, sequence, kmerLength};

    WinLockGuard lock(tasksMutex);

#ifdef _WIN32
    AsyncTask at;
    at.hThread = CreateThread(NULL, 0, ScoringEngine::threadEntry, task, 0, NULL);
    if (at.hThread == NULL) {
        delete task;
        processSequenceChunk(sequence, kmerLength);
        return;
    }
    asyncTasks.push_back(std::move(at));
#else
    AsyncTask at;
    int rc = pthread_create(&at.tid, NULL, ScoringEngine::threadEntry, task);
    if (rc != 0) {
        delete task;
        processSequenceChunk(sequence, kmerLength);
        return;
    }
    at.running = true;
    asyncTasks.push_back(std::move(at));
#endif
}

void ScoringEngine::waitForAsyncTasks() {
    std::vector<AsyncTask> tasksToWait;
    {
        WinLockGuard lock(tasksMutex);
        tasksToWait = std::move(asyncTasks);
        asyncTasks.clear();
    }

    for (auto& task : tasksToWait) {
        task.get();
    }
}

std::vector<int> ScoringEngine::getBinaryFeatureVector() const {
    std::vector<int> result;
    result.reserve(featureVectorAtomics.size());
    for (const auto& atom : featureVectorAtomics) {
        result.push_back(atom.load(std::memory_order_relaxed));
    }
    return result;
}
