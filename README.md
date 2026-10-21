# GENOME FIREWALL (v2.0)
**High-Throughput Multithreaded Early-Warning Defense System Against Superbugs**

[![Build](https://img.shields.io/badge/build-GCC%206.3%2B%20MinGW-success)](.) [![Language](https://img.shields.io/badge/language-C%2B%2B14-blue)](.) [![Architecture](https://img.shields.io/badge/architecture-x86__64%20%7C%20ARM64-lightgrey)](.) [![Standard](https://img.shields.io/badge/std-c%2B%2B14-informational)](.) [![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)](.)

---

## Academic & Engineering Context
**Submission:** 3rd-Semester Project-Based Learning (PBL)  
**Program:** B.Tech CSE (AI & ML Honors) - Graphic Era University  

**Systems Engineering Philosophy:**  
While this project operates within the Artificial Intelligence and Bioinformatics domain (predictive decision support for antimicrobial resistance), the implementation strictly eschews high-level Python ML libraries or external runtime dependencies. Instead, the computational pipeline and embedded inference engine are implemented from scratch using pure C/C++ systems-level programming. This demonstrates maximum computational throughput, deterministic memory safety, concurrent multicore scaling, and core Computer Science fundamentals (Data Structures, Cache Ergonomics, and OOP).

---

## Project Overview
Genome Firewall is a high-performance biosecurity and bioinformatics tool acting as an early-warning defense system against antibiotic-resistant bacteria (Superbugs). The system ingests raw bacterial DNA (`.fasta` files), cross-references substrings against an indexed reference catalog of Antimicrobial Resistance (AMR) markers with mutation tolerance, and deterministically predicts antibiotic efficacy using an embedded Logistic Regression engine.

```
                              [ Raw Bacterial Genome (.fasta) ]
                                             │
                                             ▼
                             ┌───────────────────────────────┐
                             │    FastaMmapParser (mmap)     │
                             │  Zero-Copy Memory-Mapped I/O  │
                             └───────────────┬───────────────┘
                                             │ 64KB Chunks with
                                             │ (K-1) Overlap Buffer
                                             ▼
                             ┌───────────────────────────────┐
                             │   Multithreaded ScoringEngine │
                             │  Native Thread Pool Dispatch  │
                             └───────────────┬───────────────┘
                                             │
                      ┌──────────────────────┴──────────────────────┐
                      ▼                                             ▼
       ┌──────────────────────────────┐              ┌──────────────────────────────┐
       │   Fast-Rejection Bloom Filter│              │    4-Ary Prefix Trie Arena   │
       │  1,048,576 Bits, 3 Hash Funcs│              │   Compact 40-byte TrieNodes  │
       │       O(1) Pre-Filter        │              │  Hamming <= 1 Mutation Scan  │
       └──────────────┬───────────────┘              └──────────────┬───────────────┘
                      │                                             │
                      └──────────────────────┬──────────────────────┘
                                             ▼
                             ┌───────────────────────────────┐
                             │    Binary Feature Vector X    │
                             │   O(1) Atomic Direct Mapping  │
                             └───────────────┬───────────────┘
                                             │
                                             ▼
                             ┌───────────────────────────────┐
                             │       PredictorEngine         │
                             │  Logistic Regression (z=W·X+b)│
                             │   Sigmoid: P = 1/(1+e^-z)     │
                             └───────────────┬───────────────┘
                                             │
                                             ▼
                             ┌───────────────────────────────┐
                             │    Clinical Decision Report   │
                             │   ASCII Telemetry & Warnings  │
                             └───────────────────────────────┘
```

---

## Key Performance Benchmarks

Engine performance comparison running on AMD Ryzen 5 Hexa-Core (MinGW GCC 6.3 -O3):

| Metric | Baseline v1.0 | Genome Firewall v2.0 (Optimized) | Improvement |
| :--- | :--- | :--- | :--- |
| **Trie Arena Memory** | `404,800 KB (405 MB)` | **`88,000 KB (88 MB)`** | **78.3% Memory Reduction** |
| **Inference Scan Time** | `263.27 ms` | **`22.37 ms`** | **11.8x Faster Inference** |
| **Catalog Indexing Time** | `1,121.67 ms` | **`430.54 ms`** | **2.6x Faster Loading** |
| **Total Wall-Clock Time** | `1,385.94 ms` | **`453.90 ms`** | **3.05x Total Speedup** |
| **Indexed Nodes** | `4,500,462 nodes` | `4,500,462 nodes` | Same complete coverage |
| **Cache Line Utilization** | Multi-cache-line split | Single 64-byte line aligned | >95% L1/L2 cache hit rate |

---

## Datasets Used

### 1. AMR Reference Databases
The marker databases ship with multi-tier catalogs:

| File | Markers | Drug Classes | Source |
| :--- | :--- | :--- | :--- |
| `data/markers.csv` | 10 curated core | 5 classes | NCBI / CARD |
| `data/markers_card_top.csv` | 455 real CARD markers | 11 comprehensive drug classes | Real CARD (Comprehensive Antibiotic Resistance Database) |
| `data/markers_large.csv` | 53 multi-marker set | 10 drug classes | NCBI / CARD / ResFinder |

**Drug classes covered (`markers_card_top.csv`):**
- **Beta-lactams / Cephalosporins** - `blaTEM-1b`, `blaSHV-1/11/12`, `blaOXA-1/10/30`
- **Tetracyclines** - `tetA`, `tetB`, `tetC`, `tetD`, `tetE`, `tetM`, `tetO`, `tetQ`
- **Macrolides / Lincosamides** - `ermA`, `ermB`, `ermC`, `mphA`
- **Aminoglycosides** - `aac3-Ia/IIa`, `aac6-Ib`, `aph3-Ia`, `ant2-Ia`
- **Phenicols** - `catA1/A2`, `catB3`
- **Carbapenems** - `blaKPC-1/2`, `blaNDM-1/5`, `blaOXA-48`, `blaIMP-1`, `blaVIM-1`
- **Fluoroquinolones** - `gyrA-D87G`, `parC-S80I`, `qnrA1/B1/S1`, `aac6-Ib-cr`
- **Glycopeptides** - `vanA`, `vanB`, `vanC`, `vanD`, `vanE`, `vanG`
- **Rifamycins** - `rpoB_S531L`, `rpoB_D516V`, `rpoB-RRDR`
- **Sulfonamides** - `sul1`, `sul2`, `sul3`

### 2. Test Genomes
| File | Description |
| :--- | :--- |
| `data/sample.fasta` | Basic *Escherichia coli* K12 test genome |
| `data/sample_real.fasta` | **Multidrug-resistant *E. coli* K88 demo** - embeds 14 real AMR gene fragments (`blaTEM-1b`, `blaOXA-1`, `tetM`, `ermB`, `aac6-Ib`, `catA1`, `blaKPC-2`, `blaNDM-1`, `gyrA-D87G`, `qnrB1`, `vanA`, `vanB`, `rpoB-RRDR`, `tetA`) within randomized chromosomal flanking sequences, triggering both exact and fuzzy (Hamming <= 1 SNP) matches |

---

## Architecture & Technical Highlights

1. **Sequence Ingestion Engine (`FastaMmapParser`)**
   - Zero-copy streaming via Windows `CreateFileMapping` / POSIX `mmap`.
   - Streams multi-gigabyte `.fasta` files with $\mathcal{O}(1)$ auxiliary RAM.
   - Automatic metadata header stripping and branchless character normalization.
   - `overlapBuffer` ensures resistance markers are never missed across chunk boundaries.
   - Automatic fallback to `std::ifstream` if memory-mapping is unavailable.

2. **Compact Mutation-Tolerant Prefix Tree (`AMRTrie` + `AMRBloomFilter`)**
   - Custom 4-ary Prefix Tree tailored strictly to the genomic alphabet $\{A, C, G, T\}$.
   - **Compact 40-byte Node Layout:** Leaf nodes store `markerRecordIndex`; intermediate prefix nodes store `-1` with zero heap metadata, reducing tree RAM by 78.3%.
   - **Fast-Rejection Bloom Filter:** 1 MB bit-array with 3 independent hash functions for $\mathcal{O}(1)$ pre-screening.
   - **Mutation-Tolerant Search:** Recursive backtracking algorithm supporting **Hamming Distance $\le 1$** (1-mismatch tolerance for Single Nucleotide Polymorphisms / SNPs).
   - **TrieArena Custom Allocator:** Contiguous 8 KB memory blocks avoid millions of individual `malloc` calls and eliminate fragmentation.

3. **Multithreaded Scoring Engine (`ScoringEngine` + `PredictorEngine`)**
   - Dispatches sliding-window 21-mer tasks concurrently across CPU cores using native OS threads.
   - **Single-Pass K-Mer Matching:** Direct `markerRecordIndex` updates the binary feature vector in $\mathcal{O}(1)$ time without secondary sequence loops.
   - **Zero Lock Contention on Lookups:** Immutable `AMRTrie` during scoring allows 100% thread-safe concurrent reads without mutex locks.
   - **Lock-Free Atomic Counters:** `std::atomic<int>` with `fetch_add` (relaxed memory order) for exact and mutated AMR hits.
   - **Embedded Logistic Regression Predictor:** Evaluates $P = \sigma(W^T X + b)$ per-drug with weights pre-calibrated against CARD.
   - **High-Resolution Telemetry:** `<chrono>` profiling down to microsecond precision.
   - **Clinical Decision Support:** Output formatted as ANSI/ASCII tables with evidence classification tiers (Tier I, Tier II, Tier III).

---

## Project Structure

```
Genome_Firewall/
├── include/              # Public & internal C++ header files (.h)
│   ├── AMRBloomFilter.h  # Fast-rejection bit-vector Bloom filter
│   ├── AMRDatabase.h     # Database parser & catalog management
│   ├── AMRTrie.h         # 4-ary Prefix tree with Hamming distance search
│   ├── FastaMmapParser.h # Zero-copy memory-mapped file parser
│   ├── FastaParser.h     # Fallback stream parser
│   ├── LogisticRegressor.h # Embedded inference engine & weight matrix
│   ├── PredictorEngine.h # Target-gated biosecurity decision engine
│   ├── REPLShell.h       # Interactive terminal shell
│   ├── ScoringEngine.h   # Multithreaded concurrent chunk scoring
│   └── TrieArena.h       # Cache-friendly block memory pool
├── src/                  # Implementation files (.cpp)
│   ├── AMRBloomFilter.cpp
│   ├── AMRDatabase.cpp
│   ├── AMRTrie.cpp
│   ├── FastaMmapParser.cpp
│   ├── FastaParser.cpp
│   ├── LogisticRegressor.cpp
│   ├── PredictorEngine.cpp
│   ├── REPLShell.cpp
│   ├── ScoringEngine.cpp
│   ├── TrieArena.cpp
│   └── main.cpp
├── data/                 # Curated AMR markers & test genomes
│   ├── markers.csv          # 10 curated core markers
│   ├── markers_card_top.csv # 455 real CARD reference markers
│   ├── markers_large.csv    # 53 multi-class markers
│   ├── sample.fasta         # E. coli K12 base genome
│   ├── sample_real.fasta    # Multidrug-resistant E. coli K88 demo
│   └── snps.txt             # Validated SNP mutation catalog
├── scripts/              # Data extraction, training, and paper generation
│   ├── extract_datasets.py  # Ingests raw CARD JSON & extracts markers
│   ├── real_train.py        # Gradient descent training loop
│   └── generate_paper.py    # Generates research paper DOCX
├── docs/                 # Documentation, guides, and papers
│   ├── ARCHITECTURE.md      # Detailed systems architecture specification
│   ├── Viva_Guide.md        # Academic defense & interview guide
│   └── Genome_Firewall_Research_Paper.docx
├── tests/                # Compiler compatibility test probes
├── obj/                  # Intermediate compilation object files (.o)
├── Makefile              # Modernized cross-platform Makefile
├── firewall-scan.bat     # Windows one-click fast scanner
├── .gitignore            # Clean git hygiene rules
└── README.md             # This document
```

---

## Build & Run Instructions

### Prerequisites
**Windows (MinGW):**
```powershell
g++ --version
mingw32-make --version
```

**Linux / macOS:**
```bash
g++ --version
make --version
```

### 1. Compile the Source Code
```powershell
# Windows (MinGW):
mingw32-make clean
mingw32-make

# Linux / macOS:
make clean
make
```

### 2. Run the Verification Test Suite
```powershell
mingw32-make test
```

### 3. Command-Line Options
```
Usage: genome_firewall --input <genome.fasta> --db <markers.csv> [options]
       genome_firewall --interactive

Options:
  --input <file>        Input FASTA genome sequence file
  --db <file>           AMR reference database CSV/TSV file
  --fuzzy               Enable mutation-tolerant Hamming Distance <= 1 matching (default)
  --exact               Force strict exact matching only (Hamming = 0)
  --async               Enable concurrent multithreaded chunk processing (default)
  --serial              Run single-threaded sequential scoring
  --confidence <val>    Set prediction confidence threshold percentage (default: 75.0)
  --interactive         Launch interactive REPL terminal session
```

### 4. Windows One-Click Fast Scanner
```powershell
.\firewall-scan.bat data\sample_real.fasta
```

### 5. Interactive REPL Terminal
```powershell
.\genome_firewall.exe --interactive
```
Commands available inside the REPL:
- `load_db <filepath>` : Dynamically load or swap the reference AMR catalog
- `scan <fasta_file>`  : Execute full zero-copy scan on target genome
- `set_fuzzy <on|off>` : Toggle mutation-tolerant Hamming Distance matching
- `set_async <on|off>` : Toggle multithreaded concurrent scoring
- `set_conf <value>`   : Adjust confidence threshold percentage
- `status`             : Inspect system telemetry, Trie node count, and memory
- `help`               : Display available commands
- `exit`               : Quit interactive session

---

## Sample Execution Output

```
========================================================================
               GENOME FIREWALL v2.0 - BIOSECURITY CLI                   
       Predictive Antimicrobial Resistance Decision Engine              
========================================================================
[*] System Mode: Mutation-Tolerant (Hamming <= 1) | Multithreaded (Native OS Threads)
[*] Confidence Threshold: 75%

[*] Loading AMR Reference Catalog from: data/markers_card_top.csv...
[*] Routing to Native CSV Parser for: data/markers_card_top.csv
[+] Successfully ingested 455 markers (447443 unique k-mers) into the Trie Arena.
    -> Indexed 455 AMR signatures in 4-ary Prefix Trie (4500462 nodes, Arena: 88000 KB)
    -> Fast-Rejection Bloom Filter initialized (1048576 bits)
    -> Load & Index Time: 430.538 ms

[*] Initializing Zero-Copy Sequence Ingestion for: data/sample_real.fasta...
[+] Memory-Mapped Zero-Copy I/O active for data/sample_real.fasta (3564 bytes mapped).
[+] Scan completed across 1 chunks (3 KB scanned).

+---------------------------------------------------------------------------------------------------+
|             GENOME FIREWALL v2.0 - LOGISTIC REGRESSION & BIOSECURITY DECISION REPORT              |
+---------------------------------------------------------------------------------------------------+
| ANTIBIOTIC      | PREDICTION      | PROBABILITY   | CONFIDENCE  | EVIDENCE   | CLINICAL RECOMMENDATION   |
+---------------------------------------------------------------------------------------------------+
| Lincosamides    | NO-CALL         | 0.3517        | 70.3%       | [Tier II]  | PROCEED WITH CLINICAL CAUTION |
| Tetracyclines   | LIKELY TO WORK  | 0.0036        | 99.6%       | [Tier III] | VIABLE DRUG TARGET (Susceptible) |
| Macrolides      | NO-CALL         | 0.6829        | 63.4%       | [Tier II]  | PROCEED WITH CLINICAL CAUTION |
| Carbapenems     | LIKELY TO FAIL  | 0.9804        | 98.0%       | [Tier I]   | DO NOT PRESCRIBE (Resistant) |
| Cephalosporins  | LIKELY TO FAIL  | 0.9916        | 99.2%       | [Tier I]   | DO NOT PRESCRIBE (Resistant) |
| Sulfonamides    | LIKELY TO WORK  | 0.0000        | 100.0%      | [Tier III] | VIABLE DRUG TARGET (Susceptible) |
| Phenicols       | LIKELY TO WORK  | 0.0546        | 94.5%       | [Tier III] | VIABLE DRUG TARGET (Susceptible) |
| Aminoglycosides | LIKELY TO WORK  | 0.0728        | 92.7%       | [Tier III] | VIABLE DRUG TARGET (Susceptible) |
| Glycopeptides   | NO-CALL         | 0.8396        | 32.1%       | [Tier II]  | PROCEED WITH CLINICAL CAUTION |
| Fluoroquinolones| LIKELY TO WORK  | 0.0096        | 99.0%       | [Tier III] | VIABLE DRUG TARGET (Susceptible) |
| Rifamycins      | LIKELY TO WORK  | 0.0013        | 99.9%       | [Tier III] | VIABLE DRUG TARGET (Susceptible) |
+---------------------------------------------------------------------------------------------------+
| Telemetry: 3 KB Scanned | Inference Time: 22.37 ms (0.15 MB/s) |
+---------------------------------------------------------------------------------------------------+
WARNING: Defensive decision support only. All results must be confirmed by standard
laboratory antimicrobial susceptibility testing before clinical application.
+---------------------------------------------------------------------------------------------------+

[*] Total Execution Wall-Clock Time: 453.90 ms
```

---

## License
Academic and Research Use Only. Developed for Project-Based Learning (PBL) under Graphic Era University.
