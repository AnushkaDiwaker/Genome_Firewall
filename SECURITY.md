# Security Policy

## 🛡️ Commitment to Biosecurity & Software Safety

**Genome Firewall** is a high-performance, systems-level bioinformatics tool designed as an early-warning defense system against antimicrobial resistance (AMR). Given its role in ingesting external genomic sequences (`.fasta`), executing memory-mapped zero-copy I/O, and evaluating resistance markers, ensuring robust security, memory safety, and deterministic parsing is paramount.

---

## 📋 Supported Versions

We provide active security updates, vulnerability remediation, and bug fixes for the versions listed below:

| Version | Supported | Notes |
| :--- | :---: | :--- |
| **2.0.x** | :white_check_mark: | Current active release branch (C++14 hybrid architecture) |
| **< 2.0.0** | :x: | Legacy prototype versions (unsupported) |

---

## 🔒 Reporting a Vulnerability

If you discover a security vulnerability, memory safety violation, out-of-bounds memory read/write, buffer overflow, or potential denial-of-service vector within Genome Firewall, please report it **responsibly and privately**.

### 📧 How to Report

Please **do NOT create a public GitHub issue** for undisclosed security vulnerabilities. Instead:

1. **Email the Maintainer directly:**
   - **Primary Contact:** Aryan Arora ([aryanarora26110@gmail.com](mailto:aryanarora26110@gmail.com))
   - **Subject Line:** `[SECURITY] Vulnerability Report: Genome Firewall`

2. **Or use GitHub Private Vulnerability Reporting:**
   - Navigate to the [Security Advisories tab](https://github.com/FROSTY-MUG/Genome_Firewall/security/advisories) on GitHub and click **"Report a vulnerability"**.

### 📝 What to Include

To help us triage and resolve the issue quickly, please provide as much of the following as possible:
- **Component Affected:** (e.g., `FastaMmapParser`, `AMRTrie`, `AMRBloomFilter`, `LogisticRegressor`, `REPLShell`)
- **Type of Issue:** (e.g., Buffer overread/overwrite, unhandled exception, integer overflow, memory leak, deadlock/race condition)
- **Environment:** Operating System (Windows / Linux / macOS), Compiler version (`g++`, `clang++`, MinGW), CPU architecture
- **Proof of Concept (PoC):** A minimal reproducible test case, including any malformed `.fasta` or `.csv` files
- **Impact Assessment:** Explanation of how the issue could be exploited or cause abnormal behavior
- **Proposed Fix:** (Optional) If you have identified a mitigation or patch

---

## ⏱️ Response & Disclosure Timeline

- **Initial Acknowledgment:** Within **48 to 72 hours** of receiving the report.
- **Triage & Assessment:** Within **7 business days**, confirming severity, reproducibility, and potential impact.
- **Fix & Patch Release:** Coordinated patch development and testing in a private branch.
- **Coordinated Public Disclosure:** Once a fix is verified and merged, a security advisory will be published acknowledging the reporter (if desired).

---

## 🧱 Architectural Security Considerations

When contributing to Genome Firewall, keep the following security design principles in mind:

1. **Memory-Mapped I/O Boundaries (`FastaMmapParser`):**
   - File size validation and bounds checking are mandatory before mapping memory via `CreateFileMapping` / `mmap`.
   - Ensure pointer offsets never exceed mapped view boundaries, especially during sliding-window k-mer lookups.

2. **Genomic Alphabet Validation (`charToIndex`):**
   - The 4-ary Prefix Tree strictly indexes nucleotide characters `{A, C, G, T}` (and lowercase equivalents).
   - Unknown bases (e.g., `'N'` in raw sequencing data) or invalid characters must safely return `-1` and abort branches cleanly without out-of-bounds array indexing.

3. **Concurrency & Thread Safety (`ScoringEngine`):**
   - The `AMRTrie` must remain **strictly immutable (read-only)** during multithreaded scanning.
   - Any counter updates across concurrent worker threads must utilize lock-free atomic operations (`std::atomic<int>::fetch_add`) to avoid race conditions.

4. **Resource Acquisition Is Initialization (RAII):**
   - Avoid raw pointer allocations; adhere strictly to `std::unique_ptr` and custom Arena allocators to eliminate memory leaks and use-after-free vulnerabilities.

---

Thank you for helping keep **Genome Firewall** and its bioinformatics community safe!
