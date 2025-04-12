#include <algorithm>
#include <list>
#include <queue>
#include <qca/qcaGraph.h>
#include<common/parameters.h>
#include <qca/qcaUtil.h>


QCAGraph::QCAGraph(const string &dotPath, const string &dotName): Graph(dotPath, dotName) {
#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
    fixFanout();
    fixFanin();
    balanceGraphAll();
}

void QCAGraph::updateG() {
    Graph::updateG();
    outNeighbors = std::vector<std::vector<int> >(nNodes);
    inNeighbors = std::vector<std::vector<int> >(nNodes);
    updateNeighbors();
}

void QCAGraph::updateNeighbors() {
    const int nNodes = QCAGraph::nNodes;

    for (int node = 0; node < nNodes; node++) {
        for (int neigh = 0; neigh < nNodes; neigh++) {
            if (successors[node][neigh])
                outNeighbors[node].push_back(neigh);

            if (predecessors[node][neigh])
                inNeighbors[node].push_back(neigh);
        }
    }
}

void QCAGraph::calcMatrix() {
    const int ioMax = static_cast<int>(max(inputNodes.size(), outputNodes.size()));
    const int numDesiredLevels = numLevels + 2;
    const int estimatedSide = static_cast<int>(ceil(sqrt(nNodes))) + 2;
    const int cellSqrt = max({estimatedSide, ioMax, numDesiredLevels + 2});

    nCellsSqrt = cellSqrt;
    nCells = nCellsSqrt * nCellsSqrt;
}

void QCAGraph::fixFanout() {
    unordered_map<int, vector<int> > fanouts;
    for (auto [fst, snd]: gEdges)
        fanouts[fst].push_back(snd);

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto &[fst, outs]: fanouts) {
        if (outs.size() <= 2) {
            for (int snd: outs) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<int> leaves;
        for (int snd: outs)
            leaves.push(snd);

        vector<int> dummyNodes;

        while (leaves.size() > 1) {
            int a = leaves.front();
            leaves.pop();
            int b = leaves.front();
            leaves.pop();

            int dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);

            newEdges.emplace_back(dummy, a);
            newEdges.emplace_back(dummy, b);

            leaves.push(dummy);
            dummyNodes.push_back(dummy);
        }

        int root = leaves.front();
        newEdges.emplace_back(fst, root);
    }

    gEdges = newEdges;
    nEdges = static_cast<int>(gEdges.size());
    nNodes = nextId;
    updateG();

#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

void QCAGraph::fixFanin() {
    unordered_map<int, vector<int> > fanins;
    for (auto [fst, snd]: gEdges)
        fanins[snd].push_back(fst);


    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto &[snd, ins]: fanins) {
        if (ins.size() <= 2) {
            for (int fst: ins) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<int> leaves;
        for (int fst: ins)
            leaves.push(fst);

        while (leaves.size() > 1) {
            int a = leaves.front();
            leaves.pop();
            int b = leaves.front();
            leaves.pop();

            int dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);

            newEdges.emplace_back(a, dummy);
            newEdges.emplace_back(b, dummy);

            leaves.push(dummy);
        }

        int root = leaves.front();
        newEdges.emplace_back(root, snd);
    }

    gEdges = newEdges;
    nEdges = static_cast<int>(gEdges.size());
    nNodes = nextId;
    updateG();
#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

void QCAGraph::computeLevels() {
    level.clear();
    levelSuccessors.clear();
    levelPredecessors.clear();

    unordered_map<int, int> inDegree;
    for (const auto [fst, snd]: gEdges)
        inDegree[snd]++;


    queue<int> q;
    for (int node: inputNodes) {
        q.push(node);
        level[node] = 0;
    }

    // Kahn's algorithm to compute topological levels
    while (!q.empty()) {
        const int fst = q.front();
        q.pop();
        for (int snd: adjList[fst]) {
            level[snd] = max(level[snd], level[fst] + 1);
            if (--inDegree[snd] == 0)
                q.push(snd);
        }
    }

    //pred and succ by level
    for (const auto [fst, snd]: gEdges) {
        levelSuccessors[level[fst]].push_back(snd);
        levelPredecessors[level[snd]].push_back(fst);
    }

    //find the minor level between the output nodes
    minOutputLevel = numeric_limits<int>::max();
    for (int out: outputNodes) {
        if (level.count(out)) {
            minOutputLevel = min(minOutputLevel, level[out]);
        }
    }

    //find the total levels
    int maxLevel = 0;
    for (const auto &[_, lvl]: level) {
        maxLevel = max(maxLevel, lvl);
    }
    numLevels = maxLevel + 1;

    calcMatrix();
}

// Balance entire graph by inserting dummy nodes
void QCAGraph::balanceGraphAll() {
    computeLevels();

    vector<pair<int, int> > balancedEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [src, dst]: gEdges) {
        const int levelSrc = level[src];
        const int levelDst = level[dst];

        if (levelDst == levelSrc + 1)
            balancedEdges.emplace_back(src, dst);
        else if (levelDst > levelSrc + 1) {
            int prev = src;
            for (int i = 0; i < levelDst - levelSrc - 1; ++i) {
                int dummy = nextId++;
                gNodes.push_back(dummy);
                dummyMap[dummy] = "dummy" + to_string(dummyCount++);

                balancedEdges.emplace_back(prev, dummy);
                prev = dummy;
            }
            balancedEdges.emplace_back(prev, dst);
        }
    }
    gEdges = move(balancedEdges);
    nEdges = static_cast<int>(gEdges.size());
    nNodes = nextId;

    updateG();
    computeLevels();

#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

// Insert dummy layer between nodes at level L and L+1
void QCAGraph::insertDummyLayerAtLevel(const int targetLevel) {
    computeLevels();

    vector<pair<int, int> > updatedEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [src, dst]: gEdges) {
        const int levelSrc = level[src];
        const int levelDst = level[dst];

        if (levelSrc == targetLevel && levelDst == targetLevel + 1) {
            int dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);

            updatedEdges.emplace_back(src, dummy);
            updatedEdges.emplace_back(dummy, dst);
        } else
            updatedEdges.emplace_back(src, dst);
    }

    gEdges = move(updatedEdges);
    nEdges = static_cast<int>(gEdges.size());
    nNodes = nextId;

    updateG();
    computeLevels();

    extraLayers++;
    extraLayersLevels.push_back(targetLevel);

#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

// Export graph as DOT format
void QCAGraph::exportUpGToDot(const string &filename) {
    ofstream fout(filename);
    if (!fout) {
        std::cerr << "Error while opening DOT file: " << filename << std::endl;
        return;
    }

    fout << "digraph network {\n";

    // Declara os nós com estilo
    for (const int node: gNodes) {
        fout << "  " << node;
        if (dummyMap.count(node))
            fout << " [style=dashed]";
        fout << ";\n";
    }

    // Escreve as arestas normalmente
    for (auto [src, dst]: gEdges)
        fout << "  " << src << " -> " << dst << ";\n";

    fout << "}\n";
    fout.close();
}

// Save dummy node map
void QCAGraph::saveDummyMap(const string &filename) {
    ofstream fout(filename);
    if (!fout) {
        std::cerr << "Erroe while opening the dummymap file: " << filename << std::endl;
        return;
    }
    for (const auto &[id, name]: dummyMap)
        fout << id << " " << name << "\n";

    fout.close();
    //cout << "Dummy map saved to: " << filename << endl;
}

bool QCAGraph::verifyPlacement(const vector<int> &n2c, const vector<pair<int, int> > &edges,
                               int *invalidEdgesCount) const {
    const int nCellsSqrt = QCAGraph::nCellsSqrt;

    bool allValid = true;
    int totalInvalid = 0;

    for (const auto &[src, dst]: edges) {
        const int srcCell = n2c[src];
        const int dstCell = n2c[dst];

        if (srcCell == -1 || dstCell == -1) {
            ++totalInvalid;
            allValid = false;
            continue;
        }

        const int srcX = getX(srcCell, nCellsSqrt);
        const int srcY = getY(srcCell, nCellsSqrt);
        const int dstX = getX(dstCell, nCellsSqrt);
        const int dstY = getY(dstCell, nCellsSqrt);

        const auto outputDirs = qcaGetOutputDirections(srcX, srcY);

        const bool isValidEdge = std::any_of(outputDirs.begin(), outputDirs.end(),
                                             [&](const std::pair<int, int> &dir) {
                                                 return dstX == srcX + dir.first && dstY == srcY + dir.second;
                                             });

        if (!isValidEdge) {
            ++totalInvalid;
            allValid = false;
        }

        /*bool found = false;
        for (auto [dx, dy]: outputDirs) {
            if (dstX == srcX + dx && dstY == srcY + dy) {
                found = true;
                break;
            }
        }

        if (!found) {
            ++totalInvalid;
            allValid = false;
        }*/
    }

    if (invalidEdgesCount != nullptr)
        *invalidEdgesCount = totalInvalid;

    return allValid;
}

unordered_map<string, vector<pair<int, int> > > QCAGraph::qcaGetGraphAnnotations(
    const vector<pair<int, int> > &edges,
    const vector<pair<int, int> > &convergences
) {
    unordered_map<string, vector<pair<int, int> > > annotations;
    vector<int> placed;
    // Initialization of the dictionary

    for (const auto &[cycleBegin,cycleEnd]: convergences) {
        //const int cycleBegin = cycleBegin;
        //int cycleEnd = cycleEnd;
        list<string> walkKeys;
        bool found_start = false;
        int count = 0;
        int current = -1;

        for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
            const int src = it->first;
            const int dst = it->second;

            if (!found_start && cycleBegin == dst) {
                current = src;
                string key = funcKey(to_string(current), to_string(cycleBegin));
                walkKeys.push_front(key);
                annotations[key].emplace_back(cycleEnd, count++);
                found_start = true;
            } else if (found_start && (dst == current || src == cycleEnd)) {
                //const int dst = dst;
                const string key = funcKey(to_string(current), to_string(dst));
                current = src;

                if (current != cycleEnd && dst != cycleEnd) {
                    walkKeys.push_front(key);
                    annotations[key].emplace_back(cycleEnd, count++);
                } else {
                    // Go back and update values
                    const int half = count / 2;
                    auto it2 = walkKeys.begin();
                    for (int k = 0; k < half; ++k, ++it2) {
                        auto &dic_actual = annotations[*it2];
                        for (auto &[target,annotationValue]: dic_actual) {
                            if (target == cycleEnd)
                                annotationValue = k + 1;
                        }
                    }
                    break; // Move to the next element in convergences
                }
            }
        }
    }

    return annotations;
}

vector<int> QCAGraph::getInterleavedOutputCellsQCA() const {
    const int edge = this->nCellsSqrt - 1;

    vector<int> outputCells;
    int row = edge;
    int col = edge;

    while (row >= 0 || col >= 0) {
        if (row >= 0) {
            const int cell = row * nCellsSqrt + edge;
            outputCells.push_back(cell);
            --row;
        }

        if (col != edge)
            if (col >= 0) {
                const int cell = edge * nCellsSqrt + col;
                outputCells.push_back(cell);
                --col;
            }
    }

    return outputCells;
}
