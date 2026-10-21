#ifndef AMR_BLOOM_FILTER_H
#define AMR_BLOOM_FILTER_H

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

// High-performance Bit-Vector Bloom Filter for O(1) Fast-Path Rejection
class AMRBloomFilter {
public:
    explicit AMRBloomFilter(size_t bitSize = 1048576, size_t hashCount = 3);
    ~AMRBloomFilter() = default;

    void add(const std::string& kmer);
    bool contains(const std::string& kmer) const;
    void clear();

    size_t getBitSize() const { return bits.size(); }

private:
    static inline uint64_t hash1(const std::string& kmer) {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : kmer) {
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    static inline uint64_t hash2(const std::string& kmer) {
        uint64_t hash = 0xcbf29ce484222325ULL;
        for (char c : kmer) {
            hash = (hash ^ static_cast<uint64_t>(c)) * 0x100000001b3ULL;
            hash = (hash << 13) | (hash >> 51);
        }
        return hash;
    }

    static inline uint64_t hash3(const std::string& kmer) {
        uint64_t hash = 0;
        uint64_t p = 31;
        uint64_t p_pow = 1;
        for (char c : kmer) {
            hash = (hash + (static_cast<uint64_t>(c) - 'A' + 1) * p_pow);
            p_pow = (p_pow * p);
        }
        return hash;
    }

    std::vector<bool> bits;
    size_t hashCount;
};

#endif // AMR_BLOOM_FILTER_H
