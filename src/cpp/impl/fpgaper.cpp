//
// Created by jeronimo on 02/11/24.
//

#include "fpgaper.h"

#include <utility>

FPGAPPeR::FPGAPPeR(const Graph &graph): graph(graph) {
    getInOutPos();
}

void FPGAPPeR::getInOutPos() {
    // Append positions in the first range
    for (int i = 1; i < graph.nCellsSqrt - 1; ++i) {
        possibleInOut.push_back(i);
    }

    // Append positions in the second range
    for (int i = 1; i < graph.nCellsSqrt - 1; ++i) {
        possibleInOut.push_back(i + graph.nCells - graph.nCellsSqrt);
    }

    // Append positions in the third range
    for (int i = graph.nCellsSqrt; i < graph.nCells - graph.nCellsSqrt; i += graph.nCellsSqrt) {
        possibleInOut.push_back(i);
    }

    // Append positions in the fourth range
    for (int i = graph.nCellsSqrt * 2 - 1; i < graph.nCells - 1; i += graph.nCellsSqrt) {
        possibleInOut.push_back(i);
    }
    int a = 1;
}

void FPGAPPeR::placeNodes(std::vector<int> &n2c, std::vector<int> &placement, const std::vector<int> &possible_pos,
                          const std::vector<int> &nodes) {
    int i = 0;
    while (i < nodes.size()) {
        const int n = nodes[i];
        while (true) {
            if (const int ch = choosePosition(placement, possible_pos); placement[ch] == -1) {
                placement[ch] = n;
                n2c[n] = ch;
                break;
            }
        }
        i++;
    }
}

int FPGAPPeR::choosePosition(const std::vector<int> &placement, const std::vector<int> &choices) {
    while (true) {
        auto it = std::next(choices.begin(), rand() % choices.size()); // Randomly select an element from the set
        if (const int ch = *it; placement[ch] == -1) {
            return ch;
        }
    }
}

int FPGAPPeR::calcTotalDistance(const std::vector<int> &n2c, const std::vector<std::pair<int, int> > &edges
) {
    int distance = -static_cast<int>(edges.size());

    for (const auto &e: edges) {
        const int dist = get_manhattan_distance(n2c[e.first], n2c[e.second], graph.nCellsSqrt);

        // Acumula a distância total
        distance += dist;
    }

    return distance;
}

//TODO
std::unordered_map<int, ReportData> FPGAPPeR::perSa(int nExec) {
}

std::unordered_map<int, ReportData> FPGAPPeR::perYoto(int nExec) {
    std::unordered_map<int, ReportData> reports;

    for (int exec_id = 0; exec_id < nExec; exec_id++) {
        std::vector<int> placement(graph.nCells, -1);
        std::vector<int> n2c(graph.nNodes, -1);
        std::vector<std::vector<std::vector<int> > > distCells = graph.getMeshDistances();

        int tries = 0;
        int swaps = 0;

        std::vector<std::pair<std::string, std::string> > edStr = graph.getEdgesDepthFirst();
        std::vector<std::pair<int, int> > ed = graph.getEdgesIdx(edStr);
        std::vector<int> nodes = graph.getNodesIdx((graph.inputNodesStr));

        placeNodes(n2c, placement, possibleInOut, nodes);


        for (auto [a,b]: ed) {
            if (n2c[b] != -1) {
                continue;
            }
            const int ja = n2c[a] / graph.nCellsSqrt;
            const int ia = n2c[a] % graph.nCellsSqrt;

            for (const auto &line: distCells) {
                bool placed = false;

                for (const auto &ij: line) {
                    ++tries;
                    const int ib = ia + ij[0];
                    const int jb = ja + ij[1];

                    // Define boundary and corner conditions
                    const bool outOfBounds = (ib < 0 || ib >= graph.nCellsSqrt || jb < 0 || jb >= graph.nCellsSqrt);
                    const bool isTopLeftCorner = (ib == 0 && jb == 0);
                    const bool isBottomRightCorner = (ib == graph.nCellsSqrt - 1 && jb == graph.nCellsSqrt - 1);
                    const bool isBottomLeftCorner = (ib == graph.nCellsSqrt - 1 && jb == 0);
                    const bool isTopRightCorner = (ib == 0 && jb == graph.nCellsSqrt - 1);

                    // Check if any condition is met
                    if (outOfBounds || isTopLeftCorner || isBottomRightCorner || isBottomLeftCorner ||
                        isTopRightCorner) {
                        continue;
                    }


                    int ch = ib * graph.nCellsSqrt + jb;

                    // Check if 'ch' is in possibleInOut
                    const bool chInPossibleInOut = (
                        std::find(possibleInOut.begin(), possibleInOut.end(), ch) != possibleInOut
                        .end());
                    // Check if 'b' is in inputNodesIdx or outputNodesIdx
                    const bool bIsIoNode = (std::find(graph.inputNodesIdx.begin(), graph.inputNodesIdx.end(), b) !=
                                            graph.
                                            inputNodesIdx.end() ||
                                            std::find(graph.outputNodesIdx.begin(), graph.outputNodesIdx.end(),
                                                      b) != graph.
                                            outputNodesIdx.end());

                    if (chInPossibleInOut) {
                        // 'ch' is in possible_positions
                        if (!bIsIoNode) {
                            continue;
                        }
                    } else {
                        // 'ch' is not in possible_positions
                        if (bIsIoNode) {
                            continue;
                        }
                    }

                    // Place the node if `placement[ch]` is unoccupied
                    if (placement[ch] == -1) {
                        placement[ch] = b;
                        n2c[b] = ch;
                        placed = true;
                        ++swaps;
                        break;
                    }
                }
                if (placed) {
                    break;
                }
            }
        }
        int tc = calcTotalDistance(n2c, ed);

        reports[exec_id] = ReportData(
            exec_id,
            graph.dotName,
            graph.dotPath,
            "yoto",
            tries,
            swaps,
            "DEPTH_FIRST",
            tc,
            placement,
            n2c
        );
    }
    return reports;
}

//TODO
std::unordered_map<int, ReportData> FPGAPPeR::perYott(int nExec) {
}
