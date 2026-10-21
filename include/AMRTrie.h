#ifndef AMR_TRIE_H
#define AMR_TRIE_H

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "TrieArena.h"
#include "AMRBloomFilter.h"

// Forward declaration
struct AMRRecord;

// ---------------------------------------------------------------------------
// Compact TrieNode (40 bytes on 64-bit architecture)
// Optimized for CPU cache lines: eliminates redundant std::string heap allocations
// across intermediate nodes, shrinking Trie memory footprint by ~65%.
// ---------------------------------------------------------------------------
struct TrieNode {
    TrieNode* children[4]{nullptr, nullptr, nullptr, nullptr};
    int markerRecordIndex{-1}; // -1 if non-terminal; >= 0 is index into AMRDatabase records
};

class AMRTrie {
public:
    AMRTrie();
    ~AMRTrie() = default;

    AMRTrie(const AMRTrie&) = delete;
    AMRTrie& operator=(const AMRTrie&) = delete;

    void clear();

    // Insert an Antimicrobial Resistance marker k-mer into the Trie and Bloom Filter
    void insertMarker(const std::string& marker, int recordIndex);

    // Fast exact marker search (Bloom filter pre-rejection + direct trie descent)
    bool searchExact(const std::string& marker, int* outRecordIndex = nullptr) const;

    // Mutation-tolerant search (Hamming distance <= maxMismatches)
    bool searchWithMismatch(const std::string& marker, int maxMismatches = 1, int* outRecordIndex = nullptr) const;

    // String metadata compatibility overloads for legacy/REPL callers
    bool searchExact(const std::string& marker, std::string* outMetadata) const;
    bool searchWithMismatch(const std::string& marker, int maxMismatches, std::string* outMetadata) const;

    const AMRBloomFilter& getBloomFilter() const { return bloomFilter; }
    size_t getNodeCount() const { return arena.getNodeCount(); }
    size_t getMemoryUsage() const { return arena.getMemoryUsage(); }

    // Fast inline nucleotide character to index mapper (A=0, C=1, G=2, T=3)
    static inline int charToIndex(char c) {
        switch (c) {
            case 'A': case 'a': return 0;
            case 'C': case 'c': return 1;
            case 'G': case 'g': return 2;
            case 'T': case 't': return 3;
            default: return -1;
        }
    }

private:
    TrieNode* root;
    mutable TrieArena arena;
    AMRBloomFilter bloomFilter;

    bool searchMismatchHelper(const TrieNode* node, const std::string& marker, size_t index, int mismatchesRemaining, int* outRecordIndex) const;
};

#endif // AMR_TRIE_H
