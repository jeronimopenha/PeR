#include "yotoBase.h"

#include <util.h>

#include "graph.h"

void yotoBase() {
    //std::unordered_map<int, ReportData> reports;

    //auto start = std::chrono::high_resolution_clock::now();

    std::vector<int> c2n(nCells, -1);
    std::vector<int> n2c(nNodes, -1);
    std::vector<std::vector<int> > distCells = getMeshDistances();

    int tries = 0;
    int swaps = 0;

    std::vector<std::pair<int, int> > ed = getEdgesDepthFirst();
    //saveToDot(ed, "/home/jeronimo/test.dot");

    //fixme
    placeNodes(n2c, c2n, possibleInOut, nodes);
    //
    //
    // for (auto [a,b]: ed) {
    //     if (n2c[b] != -1) {
    //         continue;
    //     }
    //     const int ja = n2c[a] / graph.nCellsSqrt;
    //     const int ia = n2c[a] % graph.nCellsSqrt;
    //
    //     for (const auto &line: distCells) {
    //         bool placed = false;
    //
    //         for (const auto &ij: line) {
    //             ++tries;
    //             const int ib = ia + ij[0];
    //             const int jb = ja + ij[1];
    //
    //             // Define boundary and corner conditions
    //             const bool outOfBounds = (ib < 0 || ib >= graph.nCellsSqrt || jb < 0 || jb >= graph.nCellsSqrt);
    //             const bool isTopLeftCorner = (ib == 0 && jb == 0);
    //             const bool isBottomRightCorner = (ib == graph.nCellsSqrt - 1 && jb == graph.nCellsSqrt - 1);
    //             const bool isBottomLeftCorner = (ib == graph.nCellsSqrt - 1 && jb == 0);
    //             const bool isTopRightCorner = (ib == 0 && jb == graph.nCellsSqrt - 1);
    //
    //             // Check if any condition is met
    //             if (outOfBounds || isTopLeftCorner || isBottomRightCorner || isBottomLeftCorner ||
    //                 isTopRightCorner) {
    //                 continue;
    //             }
    //
    //
    //             int ch = ib * graph.nCellsSqrt + jb;
    //
    //             // Check if 'ch' is in possibleInOut
    //             const bool chInPossibleInOut = (
    //                 std::find(possibleInOut.begin(), possibleInOut.end(), ch) != possibleInOut
    //                 .end());
    //             // Check if 'b' is in inputNodesIdx or outputNodesIdx
    //             const bool bIsIoNode = (std::find(graph.inputNodesIdx.begin(), graph.inputNodesIdx.end(), b) !=
    //                                     graph.
    //                                     inputNodesIdx.end() ||
    //                                     std::find(graph.outputNodesIdx.begin(), graph.outputNodesIdx.end(),
    //                                               b) != graph.
    //                                     outputNodesIdx.end());
    //
    //             if (chInPossibleInOut) {
    //                 // 'ch' is in possible_positions
    //                 if (!bIsIoNode) {
    //                     continue;
    //                 }
    //             } else {
    //                 // 'ch' is not in possible_positions
    //                 if (bIsIoNode) {
    //                     continue;
    //                 }
    //             }
    //
    //             // Place the node if `placement[ch]` is unoccupied
    //             if (c2n[ch] == -1) {
    //                 c2n[ch] = b;
    //                 n2c[b] = ch;
    //                 placed = true;
    //                 ++swaps;
    //                 break;
    //             }
    //         }
    //         if (placed) {
    //             break;
    //         }
    //     }
    // }
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double, std::milli> duration = end - start;
    // float _time = duration.count();
    // int tc = calcTotalDistance(n2c, ed);
    //
    // reports[exec_id] = ReportData(
    //     exec_id,
    //     _time,
    //     graph.dotName,
    //     graph.dotPath,
    //     "yoto",
    //     tries,
    //     swaps,
    //     "DEPTH_FIRST",
    //     tc,
    //     c2n,
    //     n2c
    // );
    // //}
    // return reports;
}

std::vector<std::vector<int> > getMeshDistances() {
    int max_dist = (nCellsSqrt - 1) * 2;
    std::vector<std::vector<int> > meshDistances;
    std::vector<std::vector<std::vector<int> > > distance_table_raw(max_dist);
    for (int i = 0; i < nCellsSqrt; ++i) {
        for (int j = 0; j < nCellsSqrt; ++j) {
            if (i == 0 && j == 0) continue; // Skip t
            const int dist = i + j;

            // Lambda to check if a coordinate pair is already in a list
            auto contains = [](const std::vector<std::vector<int> > &vec, const std::vector<int> &pair) {
                return std::find(vec.begin(), vec.end(), pair) != vec.end();
            };

            // Add unique coordinates to the distance table
            if (!contains(distance_table_raw[dist - 1], {i, j})) {
                distance_table_raw[dist - 1].push_back({i, j});
            }
            if (!contains(distance_table_raw[dist - 1], {i, -j})) {
                distance_table_raw[dist - 1].push_back({i, -j});
            }
            if (!contains(distance_table_raw[dist - 1], {-i, -j})) {
                distance_table_raw[dist - 1].push_back({-i, -j});
            }
            if (!contains(distance_table_raw[dist - 1], {-i, j})) {
                distance_table_raw[dist - 1].push_back({-i, j});
            }
        }
    }
    // Shuffle the distance table if make_shuffle is set

    auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    for (auto &d: distance_table_raw) {
        std::shuffle(d.begin(), d.end(), rng);
        for (const auto &pair: d) {
            meshDistances.push_back(pair);
        }
    }

    return meshDistances;
}
