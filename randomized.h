#include "hypergraph.h"
#include <algorithm>

class BatchTimeout : std::exception {};

struct ContractionResult {
    uint64_t numCut;
    uint64_t cutWeight;
    uint64_t totalContractions;
    uint64_t totalRuntime;
    bool success;
};

class DSU {
    std::vector<uint32_t> parent;
    std::vector<uint32_t> rank;
    uint32_t num_components;

  public:
    explicit DSU(uint32_t n) : parent(n), rank(n, 0), num_components(n) { std::iota(parent.begin(), parent.end(), 0u); }

    uint32_t find(uint32_t x) noexcept {
        uint32_t root = x;
        while (parent[root] != root) {
            root = parent[root];
        }
        while (parent[x] != root) {
            uint32_t next = parent[x];
            parent[x] = root;
            x = next;
        }
        return root;
    }

    bool unite(uint32_t a, uint32_t b) noexcept {
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        if (rank[a] < rank[b])
            std::swap(a, b);
        parent[b] = a;
        if (rank[a] == rank[b])
            ++rank[a];
        --num_components;
        return true;
    }

    uint32_t components() const noexcept { return num_components; }
};

double redoProbability(uint32_t n, uint32_t edgeSize, uint32_t k) noexcept {
    if (k <= 1)
        return 0.0;
    if (edgeSize >= n)
        return 1.0;
    if (n - edgeSize < k - 1)
        return 1.0;
    double logProd = 0.0;
    for (uint32_t i = 0; i <= k - 2; ++i) {
        logProd += std::log(static_cast<double>(n - edgeSize - i)) - std::log(static_cast<double>(n - i));
    }
    double prob = 1.0 - std::exp(logProd);
    return std::clamp(prob, 0.0, 1.0);
}

Hypergraph contractEdge(const Hypergraph &H, size_t edgeIndex) {
    DSU dsu(H.n);
    const auto &edge = H.edges[edgeIndex];
    if (!edge.vertices.empty()) {
        uint32_t first = edge.vertices[0];
        for (size_t i = 1; i < edge.vertices.size(); ++i) {
            dsu.unite(first, edge.vertices[i]);
        }
    }
    std::vector<int32_t> vertexMap(H.n, -1);
    int32_t nextId = 0;
    for (uint32_t v = 0; v < H.n; ++v) {
        uint32_t root = dsu.find(v);
        if (vertexMap[root] == -1) {
            vertexMap[root] = nextId++;
        }
        vertexMap[v] = vertexMap[root];
    }

    Hypergraph result;
    result.n = static_cast<uint32_t>(nextId);
    result.edges.reserve(H.edges.size());

    std::vector<uint32_t> newVertices;
    newVertices.reserve(H.n);

    for (size_t i = 0; i < H.edges.size(); ++i) {
        if (i == edgeIndex)
            continue;

        const auto &e = H.edges[i];
        newVertices.clear();

        for (uint32_t v : e.vertices) {
            newVertices.push_back(static_cast<uint32_t>(vertexMap[v]));
        }

        std::sort(newVertices.begin(), newVertices.end());
        newVertices.erase(std::unique(newVertices.begin(), newVertices.end()), newVertices.end());

        if (newVertices.size() > 1) {
            result.edges.emplace_back(newVertices, e.weight);
        }
    }
    return result;
}

uint32_t computeCutWeight(const std::vector<Hyperedge> &S) noexcept {
    uint32_t total = 0;
    for (const auto &e : S) {
        total += e.weight;
    }
    return total;
}

void getKSpanning(Hypergraph &H, uint32_t k, std::vector<Hyperedge> &S) {
    uint32_t threshold = (H.n >= k - 1) ? (H.n - k + 2) : 1;
    std::vector<Hyperedge> kept;
    kept.reserve(H.edges.size());
    for (auto &e : H.edges) {
        if (e.vertices.size() >= threshold) {
            S.push_back(std::move(e));
        } else {
            kept.push_back(std::move(e));
        }
    }
    H.edges = std::move(kept);
}

size_t chooseRandomEdge(const Hypergraph &H, std::mt19937_64 &rng) {
    if (H.edges.empty())
        return SIZE_MAX;

    std::vector<uint64_t> cumulative(H.edges.size());
    cumulative[0] = H.edges[0].weight;
    for (size_t i = 1; i < H.edges.size(); ++i) {
        cumulative[i] = cumulative[i - 1] + H.edges[i].weight;
    }

    std::uniform_int_distribution<uint64_t> dist(0, cumulative.back() - 1);
    uint64_t target = dist(rng);

    return static_cast<size_t>(std::lower_bound(cumulative.begin(), cumulative.end(), target + 1) - cumulative.begin());
}

std::vector<Hyperedge> branchingContract(Hypergraph H, uint32_t k, std::vector<Hyperedge> S, std::mt19937_64 &rng, uint64_t &contractions, uint64_t &runtime, uint64_t cutoff) {
    getKSpanning(H, k, S);

    if (runtime >= cutoff) {
        throw BatchTimeout();
    }

    runtime += static_cast<uint64_t>(H.edges.size()) * H.n;
    ++contractions;

    if (H.edges.empty()) {
        return S;
    }

    size_t edgeIndex = chooseRandomEdge(H, rng);
    const Hyperedge &chosen = H.edges[edgeIndex];

    double z = redoProbability(H.n, static_cast<uint32_t>(chosen.vertices.size()), k);

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng);

    Hypergraph contracted = contractEdge(H, edgeIndex);
    auto contractedCut = branchingContract(std::move(contracted), k, S, rng, contractions, runtime, cutoff);

    if (r <= z) {
        auto uncontractedCut = branchingContract(std::move(H), k, S, rng, contractions, runtime, cutoff);

        if (computeCutWeight(contractedCut) < computeCutWeight(contractedCut)) {
            return contractedCut;
        }
    }

    return contractedCut;
}

uint64_t expectedRuntime(uint64_t n, uint64_t m, uint64_t k) {
    if (k == 2) {
        double logN = std::max(1.0, std::log(static_cast<double>(n)));
        return static_cast<uint64_t>(m * n * n * std::ceil(logN));
    }
    return static_cast<uint64_t>(m * std::pow(n, 2 * k - 2));
}

ContractionResult runOnce(const Hypergraph &H, uint32_t k, uint64_t cutoff, uint64_t &contractions, uint64_t &runtime, uint64_t seed) {
    std::mt19937_64 rng(seed);

    auto edges = branchingContract(H, k, {}, rng, contractions, runtime, cutoff);

    ContractionResult result;
    result.numCut = edges.size();
    result.cutWeight = computeCutWeight(edges);
    result.totalContractions = contractions;
    result.totalRuntime = runtime;
    result.success = true;

    return result;
}

ContractionResult randomizedMinKCut(const Hypergraph &H, uint32_t k, uint64_t batchScale, uint64_t numBatchesFactor, uint64_t baseSeed, bool verbose = false) {
    uint64_t logN = static_cast<uint64_t>(std::max(1.0, std::log(static_cast<double>(H.n))));
    uint64_t T = expectedRuntime(H.n, H.edges.size(), k);
    uint64_t cutoff = 4 * batchScale * T * logN;
    uint64_t numBatches = numBatchesFactor * logN;
    uint64_t iterationsPerBatch = batchScale * logN;

    if (verbose)
        std::cout << "Expected runtime: " << T << "\n"
                  << "Batch cutoff: " << cutoff << "\n"
                  << "Iterations/batch: " << iterationsPerBatch << "\n"
                  << "Number of batches: " << numBatches << "\n";

    uint64_t totalContractions = 0;
    uint64_t totalRuntime = 0;

    std::vector<ContractionResult> batchBests;
    batchBests.reserve(numBatches);

    std::mt19937_64 seedGen(baseSeed ? baseSeed : std::random_device{}());
    std::vector<uint64_t> results;

    for (uint64_t batch = 0; batch < numBatches; batch++) {
        uint64_t batchContractions = 0;
        uint64_t batchRuntime = 0;

        ContractionResult batchBest;
        batchBest.cutWeight = std::numeric_limits<uint64_t>::max();
        bool anySuccess = false;
        for (uint64_t iter = 0; iter < iterationsPerBatch; ++iter) {
            try {
                uint64_t seed = seedGen();
                auto result = runOnce(H, k, cutoff, batchContractions, batchRuntime, seed);
                if (result.cutWeight < batchBest.cutWeight) {
                    batchBest = result;
                }
                anySuccess = true;
            } catch (const BatchTimeout &) {
                std::cout << "Batch #" << batch << " timed out at iteration " << iter << "/" << iterationsPerBatch << "\n";
                break;
            }
        }

        if (anySuccess) {
            batchBests.push_back(batchBest);
            if (verbose)
                std::cout << "Batch #" << batch << " completed in " << batchRuntime << " units (" << (100.0 * batchRuntime / cutoff) << "% of cutoff)\n";
        } else {
            std::cout << "Batch #" << batch << " failed\n";
        }

        totalContractions += batchContractions;
        totalRuntime += batchRuntime;
    }

    if (batchBests.empty()) {
        throw std::runtime_error("All batches failed to find a cut");
    }

    auto best = std::min_element(batchBests.begin(), batchBests.end(), [](const auto &a, const auto &b) { return a.cutWeight < b.cutWeight; });

    ContractionResult result = *best;
    result.totalContractions = totalContractions;
    result.totalRuntime = totalRuntime;
    return result;
}
