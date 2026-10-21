#include "AMRTrie.h"
#include <iostream>

AMRTrie::AMRTrie() : bloomFilter(1048576, 3) {
    root = arena.allocateNode();
}

void AMRTrie::clear() {
    arena.reset();
    bloomFilter.clear();
    root = arena.allocateNode();
}

void AMRTrie::insertMarker(const std::string& marker, int recordIndex) {
    if (marker.empty()) return;

    // Fast O(1) Bloom Filter addition
    bloomFilter.add(marker);

    TrieNode* current = root;
    for (char c : marker) {
        int idx = charToIndex(c);
        if (idx == -1) continue;

        if (!current->children[idx]) {
            current->children[idx] = arena.allocateNode();
        }
        current = current->children[idx];
    }
    current->markerRecordIndex = recordIndex;
}

bool AMRTrie::searchExact(const std::string& marker, int* outRecordIndex) const {
    // Fast O(1) Bloom Filter rejection check
    if (!bloomFilter.contains(marker)) {
        return false;
    }

    TrieNode* current = root;
    for (char c : marker) {
        int idx = charToIndex(c);
        if (idx == -1) return false;

        if (!current->children[idx]) {
            return false;
        }
        current = current->children[idx];
    }

    if (current && current->markerRecordIndex >= 0) {
        if (outRecordIndex) *outRecordIndex = current->markerRecordIndex;
        return true;
    }
    return false;
}

bool AMRTrie::searchWithMismatch(const std::string& marker, int maxMismatches, int* outRecordIndex) const {
    if (marker.empty() || !root) return false;
    return searchMismatchHelper(root, marker, 0, maxMismatches, outRecordIndex);
}

bool AMRTrie::searchMismatchHelper(const TrieNode* node, const std::string& marker, size_t index, int mismatchesRemaining, int* outRecordIndex) const {
    if (!node) return false;

    if (index == marker.length()) {
        if (node->markerRecordIndex >= 0) {
            if (outRecordIndex) *outRecordIndex = node->markerRecordIndex;
            return true;
        }
        return false;
    }

    char c = marker[index];
    int exactIdx = charToIndex(c);

    // Exact match branch first (highest branch probability)
    if (exactIdx >= 0 && node->children[exactIdx]) {
        if (searchMismatchHelper(node->children[exactIdx], marker, index + 1, mismatchesRemaining, outRecordIndex)) {
            return true;
        }
    }

    // Mismatch branch if budget remains
    if (mismatchesRemaining > 0) {
        for (int i = 0; i < 4; ++i) {
            if (i != exactIdx && node->children[i]) {
                if (searchMismatchHelper(node->children[i], marker, index + 1, mismatchesRemaining - 1, outRecordIndex)) {
                    return true;
                }
            }
        }
    }

    return false;
}

// Legacy string overloads
bool AMRTrie::searchExact(const std::string& marker, std::string* outMetadata) const {
    int recIdx = -1;
    bool found = searchExact(marker, &recIdx);
    if (found && outMetadata) {
        *outMetadata = std::to_string(recIdx);
    }
    return found;
}

bool AMRTrie::searchWithMismatch(const std::string& marker, int maxMismatches, std::string* outMetadata) const {
    int recIdx = -1;
    bool found = searchWithMismatch(marker, maxMismatches, &recIdx);
    if (found && outMetadata) {
        *outMetadata = std::to_string(recIdx);
    }
    return found;
}
