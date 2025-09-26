#include <algorithm>
#include <list>
#include <queue>
#include <qca/qcaGraph.h>
#include <../../include/fpga/parametersFpga.h>
#include <qca/qcaUtil.h>

using namespace std;

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
    outNeighbors = std::vector<std::vector<long> >(nNodes);
    inNeighbors = std::vector<std::vector<long> >(nNodes);
    updateNeighbors();
}

void QCAGraph::updateNeighbors() {
    const long nNodes = QCAGraph::nNodes;

    for (long node = 0; node < nNodes; node++) {
        for (long neigh = 0; neigh < nNodes; neigh++) {
            if (successors[node][neigh])
                outNeighbors[node].push_back(neigh);

            if (predecessors[node][neigh])
                inNeighbors[node].push_back(neigh);
        }
    }
}

void QCAGraph::calcMatrix() {
    const long ioMax = static_cast<long>(max(inputNodes.size(), outputNodes.size()));
    const long numDesiredLevels = numLevels + 2;
    const long estimatedSide = static_cast<long>(ceil(sqrt(nNodes))) + 2;
    const long cellSqrt = max({estimatedSide, ioMax, numDesiredLevels + 2});

    nCellsSqrt = cellSqrt;
    nCells = nCellsSqrt * nCellsSqrt;
}

void QCAGraph::fixFanout() {
    unordered_map<long, vector<long> > fanouts;
    for (auto [fst, snd]: gEdges)
        fanouts[fst].push_back(snd);

    vector<pair<long, long> > newEdges;
    long nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    long dummyCount = 0;

    for (auto &[fst, outs]: fanouts) {
        if (outs.size() <= 2) {
            for (long snd: outs) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<long> leaves;
        for (long snd: outs)
            leaves.push(snd);

        vector<long> dummyNodes;

        while (leaves.size() > 1) {
            long a = leaves.front();
            leaves.pop();
            long b = leaves.front();
            leaves.pop();

            long dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);

            newEdges.emplace_back(dummy, a);
            newEdges.emplace_back(dummy, b);

            leaves.push(dummy);
            dummyNodes.push_back(dummy);
        }

        long root = leaves.front();
        newEdges.emplace_back(fst, root);
    }

    gEdges = newEdges;
    nEdges = static_cast<long>(gEdges.size());
    nNodes = nextId;
    updateG();

#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

void QCAGraph::fixFanin() {
    unordered_map<long, vector<long> > fanins;
    for (auto [fst, snd]: gEdges)
        fanins[snd].push_back(fst);


    vector<pair<long, long> > newEdges;
    long nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    long dummyCount = 0;

    for (auto &[snd, ins]: fanins) {
        if (ins.size() <= 2) {
            for (long fst: ins) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<long> leaves;
        for (long fst: ins)
            leaves.push(fst);

        while (leaves.size() > 1) {
            long a = leaves.front();
            leaves.pop();
            long b = leaves.front();
            leaves.pop();

            long dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);

            newEdges.emplace_back(a, dummy);
            newEdges.emplace_back(b, dummy);

            leaves.push(dummy);
        }

        long root = leaves.front();
        newEdges.emplace_back(root, snd);
    }

    gEdges = newEdges;
    nEdges = static_cast<long>(gEdges.size());
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

    unordered_map<long, long> inDegree;
    for (const auto [fst, snd]: gEdges)
        inDegree[snd]++;


    queue<long> q;
    for (long node: inputNodes) {
        q.push(node);
        level[node] = 0;
    }

    // Kahn's algorithm to compute topological levels
    while (!q.empty()) {
        const long fst = q.front();
        q.pop();
        for (long snd: succList[fst]) {
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
    minOutputLevel = numeric_limits<long>::max();
    for (long out: outputNodes) {
        if (level.count(out)) {
            minOutputLevel = min(minOutputLevel, level[out]);
        }
    }

    //find the total levels
    long maxLevel = 0;
    for (const auto &[_, lvl]: level) {
        maxLevel = max(maxLevel, lvl);
    }
    numLevels = maxLevel + 1;

    calcMatrix();
}

// Balance entire graph by inserting dummy nodes
void QCAGraph::balanceGraphAll() {
    computeLevels();

    vector<pair<long, long> > balancedEdges;
    long nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    long dummyCount = 0;

    for (auto [src, dst]: gEdges) {
        const long levelSrc = level[src];
        const long levelDst = level[dst];

        if (levelDst == levelSrc + 1)
            balancedEdges.emplace_back(src, dst);
        else if (levelDst > levelSrc + 1) {
            long prev = src;
            for (long i = 0; i < levelDst - levelSrc - 1; ++i) {
                long dummy = nextId++;
                gNodes.push_back(dummy);
                dummyMap[dummy] = "dummy" + to_string(dummyCount++);

                balancedEdges.emplace_back(prev, dummy);
                prev = dummy;
            }
            balancedEdges.emplace_back(prev, dst);
        }
    }
    gEdges = move(balancedEdges);
    nEdges = static_cast<long>(gEdges.size());
    nNodes = nextId;

    updateG();
    computeLevels();

#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

// Insert dummy layer between nodes at level L and L+1
void QCAGraph::insertDummyLayerAtLevel(const long targetLevel) {
    computeLevels();

    vector<pair<long, long> > updatedEdges;
    long nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    long dummyCount = 0;

    for (auto [src, dst]: gEdges) {
        const long levelSrc = level[src];
        const long levelDst = level[dst];

        if (levelSrc == targetLevel && levelDst == targetLevel + 1) {
            long dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);

            updatedEdges.emplace_back(src, dummy);
            updatedEdges.emplace_back(dummy, dst);
        } else
            updatedEdges.emplace_back(src, dst);
    }

    gEdges = move(updatedEdges);
    nEdges = static_cast<long>(gEdges.size());
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
    for (const long node: gNodes) {
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

bool QCAGraph::verifyPlacement(const vector<long> &n2c, const vector<pair<long, long> > &edges,
                               long *invalidEdgesCount) const {
    const long nCellsSqrt = QCAGraph::nCellsSqrt;

    bool allValid = true;
    long totalInvalid = 0;

    for (const auto &[src, dst]: edges) {
        const long srcCell = n2c[src];
        const long dstCell = n2c[dst];

        if (srcCell == -1 || dstCell == -1) {
            ++totalInvalid;
            allValid = false;
            continue;
        }

        const long srcX = getColumn(srcCell, nCellsSqrt);
        const long srcY = getLine(srcCell, nCellsSqrt);
        const long dstX = getColumn(dstCell, nCellsSqrt);
        const long dstY = getLine(dstCell, nCellsSqrt);

        const auto outputDirs = qcaGetOutputDirections(srcX, srcY);

        const bool isValidEdge = std::any_of(outputDirs.begin(), outputDirs.end(),
                                             [&](const std::pair<long, long> &dir) {
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

unordered_map<string, vector<pair<long, long> > > QCAGraph::qcaGetGraphAnnotations(
    const vector<pair<long, long> > &edges,
    const vector<pair<long, long> > &convergences
) {
    unordered_map<string, vector<pair<long, long> > > annotations;
    vector<long> placed;
    // Initialization of the dictionary

    for (const auto &[cycleBegin,cycleEnd]: convergences) {
        //const long cycleBegin = cycleBegin;
        //long cycleEnd = cycleEnd;
        list<string> walkKeys;
        bool found_start = false;
        long count = 0;
        long current = -1;

        for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
            const long src = it->first;
            const long dst = it->second;

            if (!found_start && cycleBegin == dst) {
                current = src;
                string key = funcKey(to_string(current), to_string(cycleBegin));
                walkKeys.push_front(key);
                annotations[key].emplace_back(cycleEnd, count++);
                found_start = true;
            } else if (found_start && (dst == current || src == cycleEnd)) {
                //const long dst = dst;
                const string key = funcKey(to_string(current), to_string(dst));
                current = src;

                if (current != cycleEnd && dst != cycleEnd) {
                    walkKeys.push_front(key);
                    annotations[key].emplace_back(cycleEnd, count++);
                } else {
                    // Go back and update values
                    const long half = count / 2;
                    auto it2 = walkKeys.begin();
                    for (long k = 0; k < half; ++k, ++it2) {
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

vector<long> QCAGraph::getInterleavedOutputCellsQCA() const {
    const long edge = this->nCellsSqrt - 1;

    vector<long> outputCells;
    long row = edge;
    long col = edge;

    while (row >= 0 || col >= 0) {
        if (row >= 0) {
            const long cell = row * nCellsSqrt + edge;
            outputCells.push_back(cell);
            --row;
        }

        if (col != edge)
            if (col >= 0) {
                const long cell = edge * nCellsSqrt + col;
                outputCells.push_back(cell);
                --col;
            }
    }

    return outputCells;
}
