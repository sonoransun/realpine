import Foundation

// MARK: - FeatureVector

/// Sparse feature vector backed by a [String: Double] dictionary.
/// Reimplements key concepts from dicklesworthstone/fast_vector_similarity in pure Swift.
struct FeatureVector: Sendable, Equatable {
    private(set) var components: [String: Double]

    init(components: [String: Double] = [:]) {
        self.components = components
    }

    /// L2 (Euclidean) norm of the vector.
    var magnitude: Double {
        let sumOfSquares = components.values.reduce(0.0) { $0 + $1 * $1 }
        return sumOfSquares.squareRoot()
    }

    /// Returns a copy normalized to a probability distribution (values sum to 1).
    /// If all values are zero, returns an empty vector.
    func normalized() -> FeatureVector {
        let total = components.values.reduce(0.0) { $0 + abs($1) }
        guard total > 0 else { return FeatureVector() }
        var result: [String: Double] = [:]
        result.reserveCapacity(components.count)
        for (key, value) in components {
            result[key] = abs(value) / total
        }
        return FeatureVector(components: result)
    }

    /// Produces a dense array of values in the order of the supplied keys.
    /// Missing keys yield 0.0.
    func denseValues(orderedKeys: [String]) -> [Double] {
        orderedKeys.map { components[$0, default: 0.0] }
    }

    subscript(key: String) -> Double {
        get { components[key, default: 0.0] }
        set { components[key] = newValue }
    }

    var keys: Dictionary<String, Double>.Keys { components.keys }
    var isEmpty: Bool { components.isEmpty }
    var count: Int { components.count }
}

// MARK: - SimilarityMetric

enum SimilarityMetric: String, CaseIterable, Codable, Sendable {
    case cosine
    case jensenShannon
    case spearmanRank
    case hoeffdingD
}

// MARK: - SimilarityScoring

enum SimilarityScoring {

    /// Compute similarity between two feature vectors using the specified metric.
    static func score(
        _ a: FeatureVector,
        _ b: FeatureVector,
        metric: SimilarityMetric = .cosine
    ) -> Double {
        switch metric {
        case .cosine:
            return cosine(a, b)
        case .jensenShannon:
            return jensenShannon(a, b)
        case .spearmanRank:
            return spearmanRank(a, b)
        case .hoeffdingD:
            return hoeffdingD(a, b)
        }
    }

    /// Rank candidate vectors against a query, returning (index, score) pairs
    /// filtered by threshold, sorted descending, limited to `limit` results.
    static func rank(
        query: FeatureVector,
        candidates: [FeatureVector],
        metric: SimilarityMetric,
        threshold: Double,
        limit: Int
    ) -> [(index: Int, score: Double)] {
        var scored: [(index: Int, score: Double)] = []
        scored.reserveCapacity(candidates.count)

        for (idx, candidate) in candidates.enumerated() {
            let s = score(query, candidate, metric: metric)
            if s >= threshold {
                scored.append((index: idx, score: s))
            }
        }

        scored.sort { $0.score > $1.score }

        if scored.count > limit {
            scored = Array(scored.prefix(limit))
        }

        return scored
    }

    // MARK: - Cosine Similarity

    /// dot(a,b) / (|a| * |b|), operating on sparse keys efficiently.
    private static func cosine(_ a: FeatureVector, _ b: FeatureVector) -> Double {
        let magA = a.magnitude
        let magB = b.magnitude
        guard magA > 0, magB > 0 else { return 0.0 }

        // Iterate the smaller dictionary, look up in the larger
        let (smaller, larger) = a.count <= b.count ? (a, b) : (b, a)
        var dot = 0.0
        for (key, valSmall) in smaller.components {
            let valLarge = larger.components[key]
            if let valLarge {
                dot += valSmall * valLarge
            }
        }

        let result = dot / (magA * magB)
        return min(max(result, 0.0), 1.0)
    }

    // MARK: - Jensen-Shannon Divergence

    /// 1.0 - sqrt(JSD(P, Q)) where JSD is the Jensen-Shannon divergence.
    private static func jensenShannon(_ a: FeatureVector, _ b: FeatureVector) -> Double {
        let p = a.normalized()
        let q = b.normalized()

        guard !p.isEmpty || !q.isEmpty else { return 0.0 }

        // Union of all keys
        var allKeys = Set(p.keys)
        for key in q.keys {
            allKeys.insert(key)
        }

        // Compute M = (P + Q) / 2, then KL(P||M) and KL(Q||M)
        var klPM = 0.0
        var klQM = 0.0

        for key in allKeys {
            let pVal = p[key]
            let qVal = q[key]
            let mVal = (pVal + qVal) / 2.0

            guard mVal > 0 else { continue }

            if pVal > 0 {
                klPM += pVal * log(pVal / mVal)
            }
            if qVal > 0 {
                klQM += qVal * log(qVal / mVal)
            }
        }

        let jsd = 0.5 * klPM + 0.5 * klQM
        // JSD is in [0, ln(2)] for distributions; clamp to avoid floating-point issues
        let clampedJSD = min(max(jsd, 0.0), 1.0)
        return 1.0 - clampedJSD.squareRoot()
    }

    // MARK: - Spearman Rank Correlation

    /// Spearman's rho normalized to [0, 1] as (rho + 1) / 2.
    private static func spearmanRank(_ a: FeatureVector, _ b: FeatureVector) -> Double {
        var allKeys = Set(a.keys)
        for key in b.keys {
            allKeys.insert(key)
        }

        let orderedKeys = Array(allKeys)
        let n = orderedKeys.count
        guard n >= 2 else { return 0.5 }

        let aValues = a.denseValues(orderedKeys: orderedKeys)
        let bValues = b.denseValues(orderedKeys: orderedKeys)

        let ranksA = computeRanks(aValues)
        let ranksB = computeRanks(bValues)

        var sumDSquared = 0.0
        for i in 0..<n {
            let d = ranksA[i] - ranksB[i]
            sumDSquared += d * d
        }

        let nDouble = Double(n)
        let rho = 1.0 - (6.0 * sumDSquared) / (nDouble * (nDouble * nDouble - 1.0))
        // Normalize from [-1, 1] to [0, 1]
        return (rho + 1.0) / 2.0
    }

    /// Assign ranks with tie-averaging.
    private static func computeRanks(_ values: [Double]) -> [Double] {
        let n = values.count
        // Create (index, value) pairs sorted by value
        let sorted = values.enumerated()
            .sorted { $0.element < $1.element }

        var ranks = [Double](repeating: 0.0, count: n)
        var i = 0
        while i < n {
            var j = i
            // Find the extent of tied values
            while j < n - 1 && sorted[j + 1].element == sorted[i].element {
                j += 1
            }
            // Average rank for ties (ranks are 1-based)
            let avgRank = Double(i + j) / 2.0 + 1.0
            for k in i...j {
                ranks[sorted[k].offset] = avgRank
            }
            i = j + 1
        }
        return ranks
    }

    // MARK: - Hoeffding's D

    /// Hoeffding's D statistic normalized to [0, 1]. O(n^2) complexity.
    private static func hoeffdingD(_ a: FeatureVector, _ b: FeatureVector) -> Double {
        var allKeys = Set(a.keys)
        for key in b.keys {
            allKeys.insert(key)
        }

        let orderedKeys = Array(allKeys)
        let n = orderedKeys.count
        guard n >= 5 else { return 0.0 }

        let xValues = a.denseValues(orderedKeys: orderedKeys)
        let yValues = b.denseValues(orderedKeys: orderedKeys)

        let xRanks = computeRanks(xValues)
        let yRanks = computeRanks(yValues)

        let nDouble = Double(n)

        // Compute Q_i for each point: count of points (j) where
        // xRanks[j] < xRanks[i] and yRanks[j] < yRanks[i]
        var d1 = 0.0  // sum of (Q_i - 1)(Q_i - 2)
        var d2 = 0.0  // sum of (a_i - 1)(a_i - 2)(b_i - 1)(b_i - 2)
        var d3 = 0.0  // sum of (a_i - 2)(b_i - 2)(Q_i - 1)

        for i in 0..<n {
            var qi = 0  // bivariate rank: points dominated by i in both dimensions
            var ai = 0  // marginal rank count for x
            var bi = 0  // marginal rank count for y

            for j in 0..<n {
                if xRanks[j] < xRanks[i] && yRanks[j] < yRanks[i] {
                    qi += 1
                }
                if xRanks[j] < xRanks[i] {
                    ai += 1
                }
                if yRanks[j] < yRanks[i] {
                    bi += 1
                }
            }

            let qd = Double(qi)
            let ad = Double(ai)
            let bd = Double(bi)

            d1 += (qd - 1.0) * (qd - 2.0)
            d2 += (ad - 1.0) * (ad - 2.0) * (bd - 1.0) * (bd - 2.0)
            d3 += (ad - 2.0) * (bd - 2.0) * (qd - 1.0)
        }

        let a1 = d1 / (nDouble * (nDouble - 1.0) * (nDouble - 2.0) * (nDouble - 3.0))
        let a2 = d2 / (nDouble * (nDouble - 1.0) * (nDouble - 2.0) * (nDouble - 3.0) * (nDouble - 4.0))
        let a3 = d3 / (nDouble * (nDouble - 1.0) * (nDouble - 2.0) * (nDouble - 3.0))

        // Hoeffding's D formula
        let dStat = a1 - 2.0 * (nDouble - 2.0) * a3 + (nDouble - 2.0) * (nDouble - 3.0) * a2

        // D ranges from -0.5/(n-1) to 1/30 for large n.
        // Normalize: scale so max theoretical value maps to ~1.0
        // D_max ~ 1/30 for large n
        let normalized = dStat * 30.0
        return min(max(normalized, 0.0), 1.0)
    }
}
