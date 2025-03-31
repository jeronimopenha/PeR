#include <fpga/fpgaGraph.h>


FPGAGraph::FPGAGraph(const string &dotPath, const string &dotName): Graph(dotPath, dotName) {
    readNeighbors();
    calcMatrix();
    clbNodes = otherNodes;
}

void FPGAGraph::readNeighbors() {
    //neighbors vector
    neighbors.resize(nNodes);
    for (size_t i = 0; i < successors.size(); ++i) {
        for (size_t j = 0; j < successors[i].size(); ++j) {
            if (successors[i][j] || predecessors[i][j]) {
                neighbors[i].push_back(j);
            }
        }
    }
}

void FPGAGraph::calcMatrix() {
    int totalInOut = static_cast<int>(inputNodes.size() + outputNodes.size());
    int nBaseNodes = nNodes - totalInOut;
    int nCellsBaseSqrt = ceil(sqrt(nBaseNodes));
    int nBorderCells = nCellsBaseSqrt * 4;
    while (totalInOut > nBorderCells) {
        nCellsBaseSqrt += 2;
        nBorderCells = nCellsBaseSqrt * 4;
    }
    int nCellsBase = static_cast<int>(pow(nCellsBaseSqrt, 2));
    int totalCells = nCellsBase + nBorderCells;
    nCellsSqrt = ceil(sqrt(totalCells));
    nCells = static_cast<int>(pow(nCellsSqrt, 2));
}

vector<int> FPGAGraph::getInOutPos() {
    vector<int> possibleInOut;

    // Append positions in the first range
    for (int i = 1; i < nCellsSqrt - 1; ++i) {
        possibleInOut.push_back(i);
    }

    // Append positions in the second range
    for (int i = 1; i < nCellsSqrt - 1; ++i) {
        possibleInOut.push_back(i + nCells - nCellsSqrt);
    }

    // Append positions in the third range
    for (int i = nCellsSqrt; i < nCells - nCellsSqrt; i += nCellsSqrt) {
        possibleInOut.push_back(i);
    }

    // Append positions in the fourth range
    for (int i = nCellsSqrt * 2 - 1; i < nCells - 1; i += nCellsSqrt) {
        possibleInOut.push_back(i);
    }
    return possibleInOut;
}

vector<int> FPGAGraph::getClbPos() {
    vector<int> pos;
    for (int i = 1; i < nCellsSqrt - 1; i++) {
        int start = i * nCellsSqrt + 1;
        int end = (i + 1) * nCellsSqrt - 1;
        for (int j = start; j < end; j++) {
            pos.push_back(j);
        }
    }
    return pos;
}

unordered_map<string, vector<pair<int, int> > > FPGAGraph::getGraphAnnotations(
    const vector<pair<int, int> > &edges,
    const vector<pair<int, int> > &convergences
) {
    unordered_map<string, vector<pair<int, int> > > annotations;
    vector<int> placed;
    // Initialization of the dictionary
    placed.push_back(edges[0].first);
    for (const auto &[fst, snd]: edges) {
        string key = funcKey(to_string(fst), to_string(snd));
        annotations[key] = {};
        placed.push_back(snd);

        //This code tries to make yott guide the neighbot IO nodes to be near the border
        //It is an improvement to the yott code
        bool ioNode = false;
        for (const auto &node: neighbors[snd]) {
            const bool tmp = find(inputNodes.begin(), inputNodes.end(), node) != inputNodes.end() ||
                             find(outputNodes.begin(), outputNodes.end(), node) != outputNodes.end();
            if (tmp) {
                const bool tmp2 = find(placed.begin(), placed.end(), node) != placed.end();
                if (!tmp2)
                    ioNode = true;
                else
                    int asd = 1;
            }
        }
        if (ioNode) {
            annotations[key].emplace_back(-1, -1);
        }
    }

    for (const auto &[fst,snd]: convergences) {
        const int elem_cycle_begin = fst;
        int elem_cycle_end = snd;
        list<string> walk_key;
        bool found_start = false;
        int count = 0;
        int value1 = -1;

        for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
            const int a = it->first;
            int b = it->second;

            if (elem_cycle_begin == b && !found_start) {
                value1 = a;
                string key = funcKey(to_string(value1), to_string(elem_cycle_begin));
                walk_key.push_front(key);
                annotations[key].emplace_back(elem_cycle_end, count);
                count++;
                found_start = true;
            } else if (found_start && (value1 == b || elem_cycle_end == a)) {
                const int value2 = b;
                value1 = a;
                string key = funcKey(to_string(value1), to_string(value2));

                if (value1 != elem_cycle_end && value2 != elem_cycle_end) {
                    walk_key.push_front(key);
                    annotations[key].emplace_back(elem_cycle_end, count);
                    count++;
                } else {
                    // Go back and update values
                    const int half_count = count / 2;
                    auto it = walk_key.begin();
                    for (int k = 0; k < half_count; ++k, ++it) {
                        auto &dic_actual = annotations[*it];
                        for (auto &[fst,snd]: dic_actual) {
                            if (fst == elem_cycle_end) {
                                snd = k + 1;
                            }
                        }
                    }
                    break; // Move to the next element in convergences
                }
            }
        }
    }

    return annotations;
}





