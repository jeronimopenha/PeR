#include <definitions.h>
#include <util.h>
#include <fpgaPar.h>
#include <fpgaGraph.h>
#include  <fpgaUtil.h>
#include <fpgaYoto.h>

using namespace std;

struct ClusteringResultLocal {
    long K;
    vector<long> clusterOf; // local
    vector<long> medoids; // local
    long long cut;
    long maxSize;
    double score;
};

struct BestKLocal {
    long bestK;
    vector<long> bestClusterOfLocal;
};

struct Choice2 {
    long u;
    long bestK;
    long bestD;
    long secondD;
};

//***********************************************

static long clampK(long K, long compNnode);

static BestKLocal chooseBestKForComponent(
    const vector<vector<long> > &succGlobal,
    const vector<long> &compNodes,
    const vector<char> &isIOGlobal,
    long k0
);

static ClusteringResultLocal clusterComponentKmedoids(
    const vector<vector<long> > &und,
    const vector<char> &isIOLocal,
    long K,
    long refineIters,
    double cap_slack // 10% folga
);

static long chooseMedoidApprox(
    const vector<vector<long> > &und,
    const vector<long> &clusterNodes
);

static vector<long> assignWithCapacity(
    const vector<vector<long> > &distKU, // dist[k][u]
    const vector<char> &isIOLocal,
    long sCap
);

static vector<long> pickSeedsIOFirst(
    const vector<vector<long> > &und,
    const vector<char> &isIOLocal,
    long K
);

static vector<long> bfsDist(const vector<vector<long> > &und, long src);

static vector<vector<long> > buildUndirectedLocal(
    const vector<vector<long> > &succGlobal,
    const vector<long> &compNodes,
    const vector<long> &global2Local // global->local (size N global, with -1 outside comp)
);

std::vector<long> assignWithCapacity(
    const std::vector<std::vector<long> > &dist, // dist[k][u]
    const std::vector<long> &medoid,
    const std::vector<char> &isIO,
    long sCap
);

long long cutEdges(const std::vector<std::vector<long> > &succ,
                    const std::vector<long> &clusterOf);

long maxClusterSize(const std::vector<long> &clusterOf, long K);

static long long cutEdgesLocalUnd(
    const vector<vector<long> > &und,
    const vector<long> &cluster_of
);

static long maxClusterSizeLocal(const vector<long> &cluster_of, long K);

vector<vector<long> > getComponents(FPGAGraph g);

//***********************************************

static long clampK(const long K, const long compNnode) {
    return max(2L, min(K, compNnode));
}

static BestKLocal chooseBestKForComponent(
    const vector<vector<long> > &succGlobal,
    const vector<long> &compNodes,
    const vector<char> &isIOGlobal,
    const long k0 = 24
) {
    const long Nglobal = static_cast<long>(succGlobal.size());

    // map global->local
    vector<long> global2Local(Nglobal, -1);
    for (long i = 0; i < static_cast<long>(compNodes.size()); ++i) {
        global2Local[compNodes[i]] = i;
    }

    // is_io local
    const long compNNodes = static_cast<long>(compNodes.size());
    vector<char> isIoLocal(compNNodes, 0);
    for (long i = 0; i < compNNodes; ++i) {
        isIoLocal[i] = isIOGlobal[compNodes[i]];
    }

    // und local
    const auto und = buildUndirectedLocal(succGlobal, compNodes, global2Local);

    // candidates
    const vector<long> candidates = {k0, k0 + 4, k0 + 8, k0 + 12};
    ClusteringResultLocal best{0, {}, {}, 0, 0, 1e300};

    for (long k: candidates) {
        k = clampK(k, compNNodes);
        auto res = clusterComponentKmedoids(und, isIoLocal, k, /*refine_iters*/3, /*cap_slack*/0.10);
        if (res.score < best.score)
            best = std::move(res);
    }

    return {best.K, best.clusterOf};
}

/*
// exemplo: cluster_id_global size Nglobal, inicial -1
// para cada componente:
auto best = choose_best_k_for_component(succ, comp_nodes, is_io_global, 24);
for (long li = 0; li < (long)comp_nodes.size(); ++li) {
    long u = comp_nodes[li];
    cluster_id_global[u] = offset + best.best_cluster_of_local[li]; // offset se quiser IDs únicos
}
offset += best.bestK;
*/

static ClusteringResultLocal clusterComponentKmedoids(
    const vector<vector<long> > &und,
    const vector<char> &isIOLocal,
    long K,
    const long refineIters = 3,
    const double cap_slack = 0.10 // 10% folga
) {
    const long n = static_cast<long>(und.size());
    K = min(K, n);

    const long S_cap = static_cast<long>(ceil(static_cast<double>(n) / static_cast<double>(K) * (1.0 + cap_slack)));

    // init seeds
    vector<long> medoids = pickSeedsIOFirst(und, isIOLocal, K);

    vector<long> cluster_of;
    for (long it = 0; it < refineIters; ++it) {
        // BFS from each medoid => dist[k][u]
        vector<vector<long> > dist_k_u;
        dist_k_u.reserve(K);
        for (long k = 0; k < K; ++k) {
            dist_k_u.push_back(bfsDist(und, medoids[k]));
        }

        // assign
        cluster_of = assignWithCapacity(dist_k_u, isIOLocal, S_cap);

        // build nodes per cluster
        vector<vector<long> > nodes_in_cluster(K);
        for (long u = 0; u < n; ++u) {
            long c = cluster_of[u];
            if (c >= 0) nodes_in_cluster[c].push_back(u);
        }

        // update medoids (approx)
        bool changed = false;
        for (long k = 0; k < K; ++k) {
            if (nodes_in_cluster[k].empty()) continue; // pode ocorrer se K grande; ok
            long new_m = chooseMedoidApprox(und, nodes_in_cluster[k]);
            if (new_m != medoids[k]) {
                medoids[k] = new_m;
                changed = true;
            }
        }
        if (!changed) break;
    }

    const long long cut = cutEdgesLocalUnd(und, cluster_of);
    const long maxsz = maxClusterSizeLocal(cluster_of, K);

    // score: prioriza corte, penaliza estourar alvo (S_cap)
    const double score = static_cast<double>(cut) + 1000.0 * max(0L, maxsz - S_cap);

    return {K, cluster_of, medoids, cut, maxsz, score};
}

static long long cutEdgesLocalUnd(
    const vector<vector<long> > &und,
    const vector<long> &cluster_of
) {
    long long cut = 0;
    const long n = static_cast<long>(und.size());
    for (long u = 0; u < n; ++u) {
        for (const long v: und[u]) {
            if (u < v && cluster_of[u] != cluster_of[v])
                cut++;
        }
    }
    return cut;
}

static long maxClusterSizeLocal(const vector<long> &cluster_of, const long K) {
    vector<long> sz(K, 0);
    for (const long c: cluster_of)
        if (c >= 0)
            sz[c]++;
    return *max_element(sz.begin(), sz.end());
}

static long chooseMedoidApprox(
    const vector<vector<long> > &und,
    const vector<long> &clusterNodes
) {
    // candidatos = top por grau + amostra
    long n = (long) und.size();
    vector<pair<long, long> > cand; // (deg, node)
    cand.reserve(clusterNodes.size());

    for (long u: clusterNodes) cand.emplace_back(static_cast<long>(und[u].size()), u);
    sort(cand.begin(), cand.end(), [&](auto &a, auto &b) { return a.first > b.first; });

    vector<long> candidates;
    const long take_top = min(12L, static_cast<long>(cand.size()));
    for (long i = 0; i < take_top; ++i) candidates.push_back(cand[i].second);

    // amostra aleatória extra
    std::mt19937 rng(12345);
    for (long t = 0; t < 16 && static_cast<long>(clusterNodes.size()) > 0; ++t) {
        long u = clusterNodes[rng() % clusterNodes.size()];
        candidates.push_back(u);
    }
    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());

    long best_u = candidates[0];
    long long best_sum = LONG_MAX;

    for (const long u: candidates) {
        auto dist = bfsDist(und, u);
        long long s = 0;
        for (long v: clusterNodes) {
            long d = dist[v];
            if (d == LONG_MAX) { s += 1000000000LL; } // desconexo dentro comp não deve ocorrer
            else s += d;
        }
        if (s < best_sum) {
            best_sum = s;
            best_u = u;
        }
    }
    return best_u;
}

static vector<long> assignWithCapacity(
    const vector<vector<long> > &distKU, // dist[k][u]
    const vector<char> &isIOLocal,
    long sCap
) {
    const long K = static_cast<long>(distKU.size());
    const long n = static_cast<long>(distKU[0].size());

    vector<long> cluster_of(n, -1);
    vector<long> load(K, 0);

    auto best_with_capacity = [&](long u) -> long {
        long chosen = -1;
        long bestd = LONG_MAX;
        for (long k = 0; k < K; ++k) {
            if (load[k] >= sCap) continue;
            long d = distKU[k][u];
            if (d < bestd) {
                bestd = d;
                chosen = k;
            }
        }
        return chosen;
    };

    // 1) IO primeiro
    for (long u = 0; u < n; ++u) {
        if (!isIOLocal[u]) continue;
        long k = best_with_capacity(u);
        if (k >= 0) {
            cluster_of[u] = k;
            load[k]++;
        }
    }

    // 2) Ordena os demais por ambiguidade (best vs second best)
    vector<Choice2> items;
    items.reserve(n);

    for (long u = 0; u < n; ++u) {
        if (isIOLocal[u]) continue;
        long best_k = -1, best_d = LONG_MAX, second_d = LONG_MAX;
        for (long k = 0; k < K; ++k) {
            long d = distKU[k][u];
            if (d < best_d) {
                second_d = best_d;
                best_d = d;
                best_k = k;
            } else if (d < second_d) { second_d = d; }
        }
        if (best_k >= 0) items.push_back({u, best_k, best_d, second_d});
    }

    sort(items.begin(), items.end(), [](const Choice2 &a, const Choice2 &b) {
        const long da = a.secondD - a.bestD;
        const long db = b.secondD - b.bestD;
        return da < db; // mais ambíguo primeiro
    });

    for (const auto &it: items) {
        const long u = it.u;
        long k = best_with_capacity(u);
        if (k >= 0) {
            cluster_of[u] = k;
            load[k]++;
        }
    }

    // Se sobrou alguém -1 (por causa de S_cap), você aumentou S_cap pouco.
    // Dá pra tratar aqui com fallback, mas ideal é S_cap >= ceil(n/K).
    return cluster_of;
}

static vector<long> pickSeedsIOFirst(
    const vector<vector<long> > &und,
    const vector<char> &isIOLocal,
    const long K
) {
    const long n = static_cast<long>(und.size());
    vector<long> deg(n);
    for (long i = 0; i < n; ++i) deg[i] = static_cast<long>(und[i].size());

    vector<long> io;
    io.reserve(n);
    for (long i = 0; i < n; ++i) if (isIOLocal[i]) io.push_back(i);

    // ordena IO por grau desc (boa âncora)
    sort(io.begin(), io.end(), [&](const long a, const long b) { return deg[a] > deg[b]; });

    vector<long> seeds;
    seeds.reserve(K);

    // 1) pega IOs primeiro
    for (long x: io) {
        if (static_cast<long>(seeds.size()) >= K) break;
        seeds.push_back(x);
    }

    // 2) completa com nós de maior grau (que não sejam seeds)
    vector<long> nodes(n);
    iota(nodes.begin(), nodes.end(), 0);
    sort(nodes.begin(), nodes.end(), [&](long a, long b) { return deg[a] > deg[b]; });

    vector<char> used(n, 0);
    for (long s: seeds) used[s] = 1;

    for (long u: nodes) {
        if (static_cast<long>(seeds.size()) >= K) break;
        if (!used[u]) {
            used[u] = 1;
            seeds.push_back(u);
        }
    }

    // Se K > n, ajusta
    if (static_cast<long>(seeds.size()) > n) seeds.resize(n);
    return seeds;
}


static vector<long> bfsDist(const vector<vector<long> > &und, long src) {
    const long n = static_cast<long>(und.size());
    vector<long> dist(n, LONG_MAX);
    deque<long> q;
    dist[src] = 0;
    q.push_back(src);

    while (!q.empty()) {
        const long u = q.front();
        q.pop_front();
        const long du = dist[u];
        for (long v: und[u]) {
            if (dist[v] == LONG_MAX) {
                dist[v] = du + 1;
                q.push_back(v);
            }
        }
    }
    return dist;
}

static vector<vector<long> > buildUndirectedLocal(
    const vector<vector<long> > &succGlobal,
    const vector<long> &compNodes,
    const vector<long> &global2Local // global->local (size N global, with -1 outside comp)
) {
    const long nCompNodes = static_cast<long>(compNodes.size());
    vector<vector<long> > undLocal(nCompNodes);

    for (long li = 0; li < nCompNodes; ++li) {
        const long u = compNodes[li];
        for (const long v: succGlobal[u]) {
            const long lj = global2Local[v];
            if (lj >= 0) {
                undLocal[li].push_back(lj);
                undLocal[lj].push_back(li);
            }
        }
    }

    // remove duplicates
    for (auto &a: undLocal) {
        sort(a.begin(), a.end());
        a.erase(unique(a.begin(), a.end()), a.end());
    }
    return undLocal;
}


std::vector<long> assignWithCapacity(
    const std::vector<std::vector<long> > &dist, // dist[k][u]
    const std::vector<long> &medoid,
    const std::vector<char> &isIO,
    const long sCap
) {
    const long K = static_cast<long>(medoid.size());
    const long N = static_cast<long>(dist[0].size());

    std::vector<long> cluster_of(N, -1);
    std::vector<long> load(K, 0);

    auto pick_best = [&](long u) -> std::pair<long, long> {
        long bk = -1, bd = LONG_MAX;
        for (long k = 0; k < K; ++k) {
            long d = dist[k][u];
            if (d < bd) {
                bd = d;
                bk = k;
            }
        }
        return {bk, bd};
    };

    // 1) Atribui IO primeiro
    for (long u = 0; u < N; ++u) {
        if (!isIO[u]) continue;
        auto [bk, bd] = pick_best(u);
        if (bk < 0) continue;
        if (load[bk] < sCap) {
            cluster_of[u] = bk;
            load[bk]++;
        } else {
            // fallback: tenta o próximo melhor com capacidade
            long chosen = -1, bestd = LONG_MAX;
            for (long k = 0; k < K; ++k) {
                if (load[k] >= sCap) continue;
                long d = dist[k][u];
                if (d < bestd) {
                    bestd = d;
                    chosen = k;
                }
            }
            if (chosen >= 0) {
                cluster_of[u] = chosen;
                load[chosen]++;
            }
        }
    }

    // 2) Prepara lista de nós não-IO com "dificuldade" (best - second best)
    std::vector<Choice2> items;
    items.reserve(N);

    for (long u = 0; u < N; ++u) {
        if (isIO[u]) continue;
        long best_k = -1, best_d = LONG_MAX, second_d = LONG_MAX;
        for (long k = 0; k < K; ++k) {
            long d = dist[k][u];
            if (d < best_d) {
                second_d = best_d;
                best_d = d;
                best_k = k;
            } else if (d < second_d) {
                second_d = d;
            }
        }
        if (best_k >= 0) items.push_back({u, best_k, best_d, second_d});
    }

    // Quanto menor (second-best - best), mais ambíguo → atribui primeiro
    std::sort(items.begin(), items.end(), [](const Choice2 &a, const Choice2 &b) {
        const long da = a.secondD - a.bestD;
        const long db = b.secondD - b.bestD;
        return da < db;
    });

    // 3) Atribui restante respeitando capacidade
    for (const auto &it: items) {
        const long u = it.u;

        long chosen = -1;
        long bestd = LONG_MAX;

        // tenta na ordem de menor distância
        for (long k = 0; k < K; ++k) {
            if (load[k] >= sCap) continue;
            long d = dist[k][u];
            if (d < bestd) {
                bestd = d;
                chosen = k;
            }
        }

        if (chosen >= 0) {
            cluster_of[u] = chosen;
            load[chosen]++;
        }
    }

    return cluster_of;
}


long long cutEdges(const std::vector<std::vector<long> > &succ,
                    const std::vector<long> &clusterOf) {
    long long cut = 0;
    const long N = static_cast<long>(succ.size());
    for (long u = 0; u < N; ++u) {
        const long cu = clusterOf[u];
        for (const long v: succ[u]) {
            if (cu != clusterOf[v]) cut++;
        }
    }
    return cut;
}

long maxClusterSize(const std::vector<long> &clusterOf, long K) {
    std::vector<long> sz(K, 0);
    for (const long c: clusterOf) if (c >= 0) sz[c]++;
    return *std::max_element(sz.begin(), sz.end());
}

vector<vector<long> > getComponents(FPGAGraph g) {
    vector<vector<long> > components;
    vector<char> visited(g.nNodes, 0);
    queue<long> q;

    for (long start = 0; start < g.nNodes; ++start) {
        if (visited[start]) continue;

        vector<long> comp;
        visited[start] = 1;
        q.push(start);

        while (!q.empty()) {
            long u = q.front();
            q.pop();
            comp.push_back(u);

            // preds and succs
            for (long v: g.predList[u]) {
                if (!visited[v]) {
                    visited[v] = 1;
                    q.push(v);
                }
            }
            for (long v: g.succList[u]) {
                if (!visited[v]) {
                    visited[v] = 1;
                    q.push(v);
                }
            }
        }

        components.push_back(std::move(comp));
    }
    return components;
}

#define N_CLUSTERES 16.0

/*
* long K = 16

vector<long> medoid(K)

vector<long> cluster_of(N, -1)

vector<long> cap(K) e vector<long> load(K,0)
 */

void medoids(const vector<vector<long> > &components, FPGAGraph g) {
    long aMax = ceil(static_cast<double>(g.nCells) / N_CLUSTERES);
    for (const auto &component: components) {
        long nK = static_cast<long>(component.size()) / aMax;
    }
}


//Choose the parameters in fpgaPar.h definitions
int main() {
    const string rootPath = verifyPath(getProjectRoot());

    cout << rootPath << endl;

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        // Reading graphs
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));
        vector<vector<long> > components = getComponents(g);

        const long N = static_cast<long>(g.succList.size());
        vector<long> cluster_id_global(N, -1);

        long offset = 0; // for unique global idx

        vector<char> is_io_global(g.nNodes, 0);

        for (const auto node: g.inputNodes) {
            is_io_global[node] = 1;
        }

        for (const auto node: g.outputNodes) {
            is_io_global[node] = 1;
        }

        for (const auto &comp_nodes: components) {
            auto best = chooseBestKForComponent(g.succList, comp_nodes, is_io_global, 24);

            for (long li = 0; li < (long) comp_nodes.size(); ++li) {
                long u = comp_nodes[li];
                cluster_id_global[u] = offset + best.bestClusterOfLocal[li];
            }

            offset += best.bestK;
        }

        medoids(components, g);

        /*vector<vector<long> > slices = getSlices(g);

        const auto it = std::max_element(
            slices.begin(),
            slices.end(),
            [](const auto &a, const auto &b) {
                return a.size() < b.size();
            }
        );

        size_t index = std::distance(slices.begin(), it);
        size_t size = it->size();*/

        // reports vector
        vector<ReportData> reports;


        //fixme The costs functions are not working well

        auto comp = [](const ReportData &a, const ReportData &b) {
            return a.totalCost < b.totalCost;
        };


#ifndef DEBUG
        //openmp Parallelization for release execution
        long nThreads = max(1, omp_get_num_procs());
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
#endif

        for (long exec = 0; exec < nExec; exec++) {
            ReportData report;

            //defining which algorithm will be run
#if defined(YOTO_DF) || defined(YOTO_DF_PRIO) || defined(YOTO_ZZ)||defined(YOTO_DF_HY)
            report = fpgaYoto(g);
#elif  defined(YOTT) || defined(YOTT_IO)
            report = fpgaYott(g);
#elifdef SA
            report = fpgaSa(g);
#endif

#ifndef DEBUG
#pragma omp critical
#endif
            {
                if (reports.size() < 10 || report.totalCost < reports.back().totalCost) {
                    // look for the right insertion position
                    auto pos = std::lower_bound(reports.begin(), reports.end(), report, comp);
                    reports.insert(pos, report);

                    // if size is greather than 10, then remove the worst one. The last onde
                    if (reports.size() > 10)
                        reports.pop_back();
                }
            }
        }
#ifndef DEBUG
        }
#endif

#ifdef BEST_ONLY
        for (long i = 0; i < 1; i++) {
#else
        const long limit = min(10L, static_cast<long>(reports.size()));
        for (long i = 0; i < limit; i++) {
#endif
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            cout << g.dotName << endl;
            string fileName = g.dotName + "_" + to_string(i);

            //save reports for the 10 better placements
            WriteReports(rootPath, fileName, reports[i]);

#if !defined(USE_CACHE)
            //generate reports and files for vpr
#ifdef VPR_V5
            WriteVpr5Data(rootPath, fileName, reports[i], g);
#elifdef VPR_V9
            WriteVpr9Data(rootPath, fileName, reports[i], g);
#endif

            /*
        * std::string folderPath = "/home/jeronimo/GIT/PeR/reports/fpga/EPFL/yoto_df_x1_debug/metrics/";
        std::string command = "python3 script.py \"" + folderPath + "\"";
        long result = std::system(command.c_str());
        return result;
             */
#endif
        }
    }
    return 0;
}


/*
#define SLICE_DELTA 4.0

vector<vector<long> > getSlices(FPGAGraph g) {
    //find the greater ASAP
    long mAsap = *max_element(g.asap.begin(), g.asap.end());
    vector<vector<long> > slices(ceil(static_cast<double>(mAsap) / SLICE_DELTA));

    for (auto node: g.gNodes) {
        const long sliceId = g.asap[node] / SLICE_DELTA;
        slices[sliceId].push_back(node);
    }
    return slices;
}
*/
