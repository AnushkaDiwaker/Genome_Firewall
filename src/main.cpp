#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include "AMRTrie.h"
#include "AMRDatabase.h"
#include "FastaMmapParser.h"
#include "ScoringEngine.h"
#include "PredictorEngine.h"
#include "REPLShell.h"

void printBanner() {
    std::cout << "========================================================================\n";
    std::cout << "               GENOME FIREWALL v2.0 - BIOSECURITY CLI                   \n";
    std::cout << "       Predictive Antimicrobial Resistance Decision Engine              \n";
    std::cout << "========================================================================\n";
}

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " [options]\n";
    std::cerr << "       " << progName << " --interactive   (or run with no arguments for interactive terminal)\n";
    std::cerr << "       " << progName << " --scan <genome.fasta> [--db <markers.csv>] [options]\n";
    std::cerr << "       " << progName << " <genome.fasta> [<markers.csv>]\n";
    std::cerr << "Options:\n";
    std::cerr << "  --scan <file>, --input, -in   Input FASTA genome sequence file to scan\n";
    std::cerr << "  --db <file>, -d               AMR reference database CSV/TSV file (default: data/markers_card_top.csv)\n";
    std::cerr << "  --fuzzy                       Enable mutation-tolerant Hamming Distance <= 1 matching (default)\n";
    std::cerr << "  --exact                       Force strict exact matching only (Hamming = 0)\n";
    std::cerr << "  --async                       Enable concurrent multithreaded chunk processing (default)\n";
    std::cerr << "  --serial                      Run single-threaded sequential scoring\n";
    std::cerr << "  --confidence <val>            Set prediction confidence threshold percentage (default: 75.0)\n";
    std::cerr << "  --interactive, -i             Launch direct interactive terminal test system\n";
    std::cerr << "  --help, -h                    Show this usage guide\n\n";
}

int main(int argc, char* argv[]) {
    // If no arguments provided, launch direct interactive terminal test system immediately
    if (argc == 1) {
        REPLShell repl;
        repl.run();
        return 0;
    }

    std::string fastaFile = "data/sample_real.fasta";
    if (!std::ifstream(fastaFile).good()) {
        fastaFile = "sample_real.fasta";
    }
    std::string amrDbFile = "data/markers_card_top.csv";
    if (!std::ifstream(amrDbFile).good()) {
        amrDbFile = "markers_card_top.csv";
        if (!std::ifstream(amrDbFile).good()) {
            amrDbFile = "data/markers_large.csv";
            if (!std::ifstream(amrDbFile).good()) {
                amrDbFile = "markers_large.csv";
            }
        }
    }
    bool enableFuzzy = true;
    bool enableAsync = true;
    bool interactiveMode = false;
    double confidenceThreshold = 75.0;

    std::vector<std::string> positionalArgs;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--interactive" || arg == "-i") {
            interactiveMode = true;
        } else if ((arg == "--input" || arg == "-in" || arg == "--scan" || arg == "-s") && i + 1 < argc) {
            fastaFile = argv[++i];
        } else if ((arg == "--db" || arg == "-d") && i + 1 < argc) {
            amrDbFile = argv[++i];
        } else if (arg == "--exact") {
            enableFuzzy = false;
        } else if (arg == "--fuzzy") {
            enableFuzzy = true;
        } else if (arg == "--serial") {
            enableAsync = false;
        } else if (arg == "--async") {
            enableAsync = true;
        } else if (arg == "--confidence" && i + 1 < argc) {
            confidenceThreshold = std::stod(argv[++i]);
        } else if (arg[0] != '-') {
            positionalArgs.push_back(arg);
        }
    }

    if (positionalArgs.size() >= 1) {
        fastaFile = positionalArgs[0];
    }
    if (positionalArgs.size() >= 2) {
        amrDbFile = positionalArgs[1];
    }

    // Auto-resolve paths inside data/ directory if not found in current directory
    if (!std::ifstream(fastaFile).good() && std::ifstream("data/" + fastaFile).good()) {
        fastaFile = "data/" + fastaFile;
    }
    if (!std::ifstream(amrDbFile).good() && std::ifstream("data/" + amrDbFile).good()) {
        amrDbFile = "data/" + amrDbFile;
    }

    if (interactiveMode) {
        REPLShell repl;
        repl.run();
        return 0;
    }

    printBanner();
    std::cout << "[*] System Mode: " << (enableFuzzy ? "Mutation-Tolerant (Hamming <= 1)" : "Exact Match Only (Hamming = 0)")
              << " | " << (enableAsync ? "Multithreaded (Native OS Threads)" : "Sequential (Single-threaded)") << "\n";
    std::cout << "[*] Confidence Threshold: " << confidenceThreshold << "%\n\n";

    auto totalStart = std::chrono::high_resolution_clock::now();

    try {
        // 1. Initialize Trie & Database
        AMRTrie trie;
        AMRDatabase db;

        std::cout << "[*] Loading AMR Reference Catalog from: " << amrDbFile << "...\n";
        auto dbStart = std::chrono::high_resolution_clock::now();
        db.loadFromFile(amrDbFile, trie);
        auto dbEnd = std::chrono::high_resolution_clock::now();
        double dbTimeMs = std::chrono::duration<double, std::milli>(dbEnd - dbStart).count();

        std::cout << "    -> Indexed " << db.getMarkerCount() << " AMR signatures in 4-ary Prefix Trie ("
                  << trie.getNodeCount() << " nodes, Arena: " << (trie.getMemoryUsage() / 1024) << " KB)\n";
        std::cout << "    -> Fast-Rejection Bloom Filter initialized (" << trie.getBloomFilter().getBitSize() << " bits)\n";
        std::cout << "    -> Load & Index Time: " << std::fixed << std::setprecision(3) << dbTimeMs << " ms\n\n";

        std::cout << "[*] Initializing Zero-Copy Sequence Ingestion for: " << fastaFile << "...\n";
        FastaMmapParser parser(fastaFile);
        ScoringEngine engine(trie, db, enableFuzzy, confidenceThreshold);

        // Render visual ASCII DNA Helix scanning animation
        PredictorEngine::renderAsciiScanAnimation(fastaFile, parser.getFileSize());

        const size_t KMER_LENGTH = 21;
        const size_t CHUNK_SIZE = 4096;
        const size_t OVERLAP_SIZE = KMER_LENGTH - 1;

        std::string chunk;
        size_t totalBytesParsed = 0;
        size_t chunkCount = 0;

        auto scanStart = std::chrono::high_resolution_clock::now();

        while (parser.getNextChunk(chunk, CHUNK_SIZE, OVERLAP_SIZE)) {
            chunkCount++;
            totalBytesParsed += chunk.size();

            if (enableAsync) {
                engine.processSequenceChunkAsync(chunk, KMER_LENGTH);
            } else {
                engine.processSequenceChunk(chunk, KMER_LENGTH);
            }
        }

        if (enableAsync) {
            engine.waitForAsyncTasks();
        }

        auto scanEnd = std::chrono::high_resolution_clock::now();
        double scanTimeMs = std::chrono::duration<double, std::milli>(scanEnd - scanStart).count();

        std::cout << "[+] Scan completed across " << chunkCount << " chunks (" 
                  << (totalBytesParsed / 1024) << " KB scanned).\n";

        // 3. Extract Binary Feature Vector X & Evaluate Logistic Regression Predictions
        std::vector<int> X = engine.getBinaryFeatureVector();
        auto predictions = engine.getPredictor().evaluatePredictionsWithModel(db, X);
        std::string report = engine.getPredictor().generateASCIIReport(predictions, scanTimeMs, totalBytesParsed);
        std::cout << report;

        // 4. Generate & Display Biosecurity Trust Score Card
        TrustScoreMetrics trustMetrics = engine.getPredictor().computeTrustScore(
            predictions, engine.getExactHits(), engine.getMutatedHits(), X, totalBytesParsed);
        std::cout << engine.getPredictor().generateTrustScoreCard(trustMetrics);

        auto totalEnd = std::chrono::high_resolution_clock::now();
        double totalTimeMs = std::chrono::duration<double, std::milli>(totalEnd - totalStart).count();
        std::cout << "[*] Total Execution Wall-Clock Time: " << std::fixed << std::setprecision(2) << totalTimeMs << " ms\n\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[!] FATAL ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
