#ifndef PREDICTOR_ENGINE_H
#define PREDICTOR_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "AMRTrie.h"
#include "AMRDatabase.h"
#include "LogisticRegressor.h"

enum class PredictionStatus {
    LIKELY_TO_FAIL, // P > 0.85 (High confidence of resistance)
    LIKELY_TO_WORK, // P < 0.15 AND Target Intact (High confidence of susceptibility)
    NO_CALL         // 0.15 <= P <= 0.85 (Uncertain evidence / statistical association)
};

enum class EvidenceTier {
    TIER_I,   // Direct match to validated AMR gene or point mutation
    TIER_II,  // Statistical association score based on multi-marker profile
    TIER_III  // Absence of resistance signals coupled with confirmed molecular target integrity
};

struct DrugPrediction {
    std::string drugName;
    PredictionStatus status;
    double probability;     // Logistic regression probability P (0.0 to 1.0)
    double confidenceScore; // Scaled confidence percentage (0.0 to 100.0%)
    EvidenceTier tier;
    int directHits{0};
    int mutatedHits{0};
    bool targetPresent{false};
    std::string clinicalRecommendation;
};

struct TrustScoreMetrics {
    double overallScore{0.0};       // 0.0 to 100.0%
    double sequenceFidelity{0.0};   // Ratio of exact hits vs mutated hits
    double kmerDepthQuality{0.0};   // K-mer coverage quality
    double decisionMargin{0.0};     // Model entropy / certainty margin
    double signalToNoiseRatio{0.0}; // Empirical SNR in dB
    double falseDiscoveryRate{0.0}; // Estimated FDR
    int exactHits{0};
    int mutatedHits{0};
    int totalActiveFeatures{0};
    std::string clinicalVerdict;
    std::string starsRating;
};

class PredictorEngine {
public:
    explicit PredictorEngine(const AMRTrie& trie, double confidenceThreshold = 75.0);

    // Module 01: Sequence Feature Extraction (legacy/fallback path)
    void analyzeSequenceFeatures(const std::string& sequence, size_t kmerLength, bool enableFuzzy);

    // Direct thread-safe hit accumulator (avoids redundant second sequence pass)
    void mergeHits(const std::unordered_map<std::string, int>& direct,
                   const std::unordered_map<std::string, int>& mutated,
                   const std::unordered_map<std::string, bool>& targetIntegrity,
                   int directCount, int mutatedCount);

    // Module 02 & 03: Evaluate target-gated predictions using Logistic Regression
    std::vector<DrugPrediction> evaluatePredictionsWithModel(const AMRDatabase& db, const std::vector<int>& featureVector) const;

    std::string generateASCIIReport(const std::vector<DrugPrediction>& predictions, double elapsedMs, size_t totalBytesScanned) const;

    // Biosecurity Trust Score computation & ASCII Card
    TrustScoreMetrics computeTrustScore(const std::vector<DrugPrediction>& predictions,
                                        int exactHits, int mutatedHits,
                                        const std::vector<int>& featureVector,
                                        size_t totalBytesScanned) const;

    std::string generateTrustScoreCard(const TrustScoreMetrics& metrics) const;

    // Visual ASCII DNA Helix & Scanning Telemetry Animation
    static void renderAsciiScanAnimation(const std::string& targetName, size_t fileSizeBytes);

    const LogisticRegressor& getModel() const { return regressor; }

    int getDirectHitCount() const { return directHitCount; }
    int getMutatedHitCount() const { return mutatedHitCount; }

private:
    const AMRTrie& trie;
    double confidenceThreshold;
    mutable LogisticRegressor regressor;

    int directHitCount{0};
    int mutatedHitCount{0};
    std::unordered_map<std::string, int> drugDirectMatches;
    std::unordered_map<std::string, int> drugMutatedMatches;
    std::unordered_map<std::string, bool> drugTargetIntegrity;

    std::string statusToString(PredictionStatus status) const;
    std::string tierToString(EvidenceTier tier) const;
};

#endif // PREDICTOR_ENGINE_H
