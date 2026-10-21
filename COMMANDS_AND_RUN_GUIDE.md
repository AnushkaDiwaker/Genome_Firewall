# Genome Firewall v2.0: Complete Commands & Execution Guide
**Target System:** Windows (PowerShell / CMD) & Linux / macOS  
**Project:** Genome Firewall — Predictive Biosecurity & Antimicrobial Resistance Engine  

---

## Quick Reference: Top 5 Commands to Run Right Now

```powershell
# 1. Compile the Project:
make

# 2. Launch Direct Interactive Terminal (Default):
.\genome_firewall.exe

# 3. Batch Scan with Full 455-Gene CARD Reference Database:
.\genome_firewall.exe --input data/sample_real.fasta --db data/markers_card_top.csv

# 4. Windows One-Click Fast Scanner:
.\firewall-scan.bat data\sample_real.fasta

# 5. Run Verification Test Suite:
make test
```

---

# Table of Contents
1. [Prerequisites & Compiler Toolchain](#1-prerequisites--compiler-toolchain)
2. [How to Build & Compile the Project](#2-how-to-build--compile-the-project)
3. [How to Run the Full Project (All Execution Modes)](#3-how-to-run-the-full-project-all-execution-modes)
4. [How to Ingest a Sample FASTA File](#4-how-to-ingest-a-sample-fasta-file)
5. [How to Download, Import & Run External Genomic Datasets (including 2 GB+ Files)](#5-how-to-download-import--run-external-genomic-datasets-including-2-gb-files)
6. [How to Inspect Diagnostic Scores, Memory Maps & Trie Queries](#6-how-to-inspect-diagnostic-scores-memory-maps--trie-queries)
7. [How to Re-extract CARD Datasets and Retrain ML Weights](#7-how-to-re-extract-card-datasets-and-retrain-ml-weights)
8. [Automated Headless & Pipeline Testing](#8-automated-headless--pipeline-testing)

---

## 1. Prerequisites & Compiler Toolchain

Genome Firewall is written in standard **C++14** with zero external runtime dependencies.

### Windows (MinGW / MSYS2)
If `make` or `g++` are not in your default system PATH, set them in PowerShell:
```powershell
$env:Path = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;" + $env:Path
```
Verify compiler availability:
```powershell
g++ --version
make --version
```

### Linux / macOS
```bash
sudo apt update && sudo apt install build-essential  # Debian / Ubuntu
brew install gcc make                                # macOS
```

---

## 2. How to Build & Compile the Project

```powershell
# Clean previous object files:
make clean

# Compile release build with -O3 optimizations and native threading:
make
```
This generates the standalone binary:
- **Windows:** `genome_firewall.exe` (or `genome_firewall`)
- **Linux/macOS:** `genome_firewall`

---

## 3. How to Run the Full Project (All Execution Modes)

### Mode A: Direct Interactive Terminal REPL (Recommended for Live Demo)
Running with no arguments automatically opens the interactive biosecurity terminal:
```powershell
.\genome_firewall.exe
```
Inside the interactive terminal:
* Type `menu` to view the numbered option menu `[1]–[9]`.
* Type `scan` or `1` to run a zero-copy memory-mapped scan.
* Type `demo` or `7` to immediately run the benchmark on `data/sample_real.fasta`.
* Type `status` or `8` to inspect Trie RAM allocation and Bloom filter bit count.
* Type `help` to display all command descriptions.
* Type `exit` or `9` to quit.

---

### Mode B: Direct Batch CLI Execution
Execute complete genomic screening directly from the command line:

```powershell
# Standard scan on real MDR E. coli genome against 455 CARD resistance genes:
.\genome_firewall.exe --input data/sample_real.fasta --db data/markers_card_top.csv

# Force strict exact matching only (Hamming = 0, no mutation tolerance):
.\genome_firewall.exe --input data/sample_real.fasta --db data/markers_card_top.csv --exact

# Single-threaded sequential mode (useful for step-by-step debugging):
.\genome_firewall.exe --input data/sample_real.fasta --db data/markers_card_top.csv --serial

# Adjust decision confidence threshold to 85.0%:
.\genome_firewall.exe --input data/sample_real.fasta --db data/markers_card_top.csv --confidence 85.0
```

### All Available CLI Flags:
```
Options:
  --input <file>, -in   Input FASTA genome sequence file
  --db <file>, -d       AMR reference database CSV/TSV file
  --fuzzy               Enable mutation-tolerant Hamming Distance <= 1 matching (default)
  --exact               Force strict exact matching only (Hamming = 0)
  --async               Enable concurrent multithreaded chunk processing (default)
  --serial              Run single-threaded sequential scoring
  --confidence <val>    Set prediction confidence threshold percentage (default: 75.0)
  --interactive, -i     Launch direct interactive terminal test system
  --help, -h            Show usage guide
```

---

## 4. How to Ingest a Sample FASTA File

### Method 1: Using the Interactive Terminal
1. Launch the executable:
   ```powershell
   .\genome_firewall.exe
   ```
2. Enter the `scan` command with your file:
   ```
   genome_firewall> scan data/sample_real.fasta
   ```
   *(Note: If you omit `data/`, the engine automatically searches both the root folder and the `data/` subfolder).*

3. What you will see:
   * **Real-time ASCII DNA Double Helix Animation** showing chromosome streaming.
   * **Virtual Memory Page Ingestion Progress** (100% complete, 0 page faults).
   * **Logistic Regression Biosecurity Report** across 11 antibiotic classes:
     - `LIKELY TO FAIL` $\to$ **DO NOT PRESCRIBE (Resistant)**
     - `LIKELY TO WORK` $\to$ **VIABLE DRUG TARGET (Susceptible)**
     - `NO-CALL` $\to$ **PROCEED WITH CLINICAL CAUTION**
   * **Biosecurity Trust Score Card** (Sequence Fidelity, K-mer Depth, Model Margin, SNR in dB, FDR).
   * **Exact vs. Mutated Hits Breakdown**.

---

### Method 2: Testing Raw DNA Sequence (No File Needed)
You can paste raw nucleotide sequences directly into the terminal:
```
genome_firewall> scan_seq
```
Paste any DNA string:
```
>test_sequence
ATGCGATCGATCGATCGATCGATCGATCGATCGATCGATCGATC
```
Press Enter on an empty line to trigger immediate 21-mer inference and get a Trust Score report.

---

## 5. How to Download, Import & Run External Genomic Datasets (including 2 GB+ Files)

### Where to Download Real Public Genomes
Public bacterial genomes can be obtained from the **National Center for Biotechnology Information (NCBI)** and **European Bioinformatics Institute (EBI)**:

#### Step 1: Download using the Official NCBI `datasets` CLI Tool (Windows)
```powershell
# 1. Download official NCBI datasets utility:
curl -o datasets.exe 'https://ftp.ncbi.nlm.nih.gov/pub/datasets/command-line/v2/win64/datasets.exe'

# 2. Download 500 complete clinical genomes of Klebsiella pneumoniae (~2.5 GB uncompressed):
.\datasets.exe download genome taxon "Klebsiella pneumoniae" --assembly-level complete --include genome --filename kp_2gb_dataset.zip

# 3. Unzip into a single combined multi-FASTA file:
tar -xf kp_2gb_dataset.zip
Get-Content ncbi_dataset/data/*/*.fna | Out-File -Encoding ascii data/my_large_genome.fasta
```

#### Step 2: Download Direct Clinical Samples via Web Browser
1. Visit NCBI Pathogen Detection:  
   `https://www.ncbi.nlm.nih.gov/pathogens/`
2. Search for any clinical pathogen (e.g. *Acinetobacter baumannii*, *Pseudomonas aeruginosa*, *Staphylococcus aureus*).
3. Click **Download** $\to$ **FASTA Genomic Sequences (.fna / .fasta)**.
4. Save the downloaded file into the `data/` folder (e.g. `data/hospital_isolate.fasta`).

---

### Step 3: Run Genome Firewall on the Downloaded File
Simply point `--input` to your downloaded FASTA file:
```powershell
.\genome_firewall.exe --input data/my_large_genome.fasta --db data/markers_card_top.csv
```

### Why Genome Firewall Handles 2 GB+ Genomes with Zero Lag:
* Traditional software copies the entire 2 GB file into RAM using `std::string`, consuming 4–8 GB and crashing with `std::bad_alloc` (Out Of Memory).
* Genome Firewall uses **OS Virtual Memory Mapping (`CreateFileMappingA`/`MapViewOfFile` & `mmap`)**.
* The operating system maps the file into 64-bit virtual address space and loads only 4,096-byte hardware pages on demand.
* **Working RAM remains strictly $\sim 20\text{ MB}$**, even when scanning a 50 GB metagenomic sequencing dataset!

---

## 6. How to Inspect Diagnostic Scores, Memory Maps & Trie Queries

### 1. Test Individual K-mers Against the Bloom Filter & 4-ary Trie
Inside the REPL terminal:
```
genome_firewall> query TTGACCGTTCGCCTGTTGACC
```
Output:
* **Bloom Filter Result:** Tests all 3 hash functions (FNV-1a, Murmur3, Polynomial).
* **Exact Trie Descent:** Checks if the 21-mer has an exact $\mathcal{O}(K)$ match.
* **Hamming Distance $\le 1$ Mutation Backtracking:** Discovers if any point mutation (SNP) matches a known resistance gene within 1 base substitution.

### 2. Audit OS Memory-Mapped Virtual Page Throughput
```
genome_firewall> mmap data/sample_real.fasta
```
Output:
* Memory-map initialization latency (in microseconds).
* Streaming throughput in **MB/s**.
* Validation of hardware page boundaries (4,096 bytes) and 20-byte boundary overlap buffer integrity.

### 3. Inspect Mathematical Feature Vector $X$
```
genome_firewall> scores
```
Displays every active binary index in $X \in \{0, 1\}^D$ mapped back to its gene name and antibiotic class.

---

## 7. How to Re-extract CARD Datasets and Retrain ML Weights

If you want to update the AMR catalogs or re-run gradient descent:

### 1. Re-extracting CARD v3.2.4 Database
```powershell
# Extracts markers from raw CARD ontology into data/markers_card_top.csv:
python scripts/extract_datasets.py
```

### 2. Retraining Logistic Regression via Gradient Descent
```powershell
# Simulates 2,000 bacterial isolates, performs 80/20 train/test split,
# runs 200 epochs of batch gradient descent (alpha=0.1),
# and generates C++ weight lookup code:
python scripts/real_train.py
```

---

## 8. Automated Headless & Pipeline Testing

You can pipe commands directly into Genome Firewall for automated CI/CD or evaluation scripts:

```powershell
# Pipe commands from data/repl_commands.txt:
Get-Content data/repl_commands.txt | .\genome_firewall.exe --interactive
```

Or pass a single-line command in PowerShell:
```powershell
"scan data/sample_real.fasta`nexit" | .\genome_firewall.exe
```

---
*Generated for Genome Firewall v2.0 project evaluation and live demonstrations.*
