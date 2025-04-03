#include <algorithm>
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

void QCAGraph::calcMatrix() {
    nCellsSqrt = static_cast<int>(ceil(sqrt(nNodes))) * 2;
    nCells = static_cast<int>(pow(nCellsSqrt, 2));
}

void QCAGraph::fixFanout() {
    unordered_map<int, vector<int> > fanouts;
    for (auto [fst, snd]: gEdges) {
        fanouts[fst].push_back(snd);
    }

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto &[fst, outs]: fanouts) {
        if (outs.size() <= 2) {
            for (int snd: outs) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<int> leaves;
        for (int snd: outs) leaves.push(snd);

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
    for (auto [fst, snd]: gEdges) {
        fanins[snd].push_back(fst);
    }

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto &[snd, ins]: fanins) {
        if (ins.size() <= 2) {
            for (int fst: ins) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<int> leaves;
        for (int fst: ins) leaves.push(fst);

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

    for (auto [fst, snd]: gEdges) {
        inDegree[snd]++;
    }

    queue<int> q;
    for (int node: inputNodes) {
        q.push(node);
        level[node] = 0;
    }

    while (!q.empty()) {
        int fst = q.front();
        q.pop();
        for (int snd: adjList[fst]) {
            level[snd] = max(level[snd], level[fst] + 1);
            if (--inDegree[snd] == 0)
                q.push(snd);
        }
    }

    for (auto [fst, snd]: gEdges) {
        levelSuccessors[level[fst]].push_back(snd);
        levelPredecessors[level[snd]].push_back(fst);
    }
    minOutputLevel = numeric_limits<int>::max();
    for (int out: outputNodes) {
        if (level.count(out)) {
            minOutputLevel = min(minOutputLevel, level[out]);
        }
    }
    calcMatrix();
}

// Balance entire graph by inserting dummy nodes
void QCAGraph::balanceGraphAll() {
    computeLevels();

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [fst, snd]: gEdges) {
        const int lfst = level[fst];
        int lsnd = level[snd];

        if (lsnd == lfst + 1) {
            newEdges.emplace_back(fst, snd);
        } else if (lsnd > lfst + 1) {
            int last = fst;
            for (int i = 0; i < lsnd - lfst - 1; ++i) {
                int dummy = nextId++;
                gNodes.push_back(dummy);
                dummyMap[dummy] = "dummy" + to_string(dummyCount++);
                newEdges.emplace_back(last, dummy);
                last = dummy;
            }
            newEdges.emplace_back(last, snd);
        }
    }
    gEdges = newEdges;
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

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [fst, snd]: gEdges) {
        const int lfst = level[fst];
        int lsnd = level[snd];

        if (lfst == targetLevel && lsnd == targetLevel + 1) {
            int dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);
            newEdges.emplace_back(fst, dummy);
            newEdges.emplace_back(dummy, snd);
        } else {
            newEdges.emplace_back(fst, snd);
        }
    }

    gEdges = newEdges;
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
    fout << "digraph network {\n";
    for (auto [fst, snd]: gEdges) {
        fout << "  " << fst << " -> " << snd;
        if (dummyMap.count(fst) || dummyMap.count(snd)) {
            fout << " [color=gray, style=dashed]";
        }
        fout << ";\n";
    }
    fout << "}\n";
    fout.close();
    //cout << "DOT file exported to: " << filename << endl;
}

// Save dummy node map
void QCAGraph::saveDummyMap(const string &filename) {
    ofstream fout(filename);
    for (auto &[id, name]: dummyMap) {
        fout << id << " " << name << "\n";
    }
    fout.close();
    cout << "Dummy map saved to: " << filename << endl;
}

bool QCAGraph::verifyPlacement(const vector<int> &n2c) {
    bool valid = true;
    for (int node = 0; node < nNodes; ++node) {
        const int nodeCell = n2c[node];
        if (nodeCell == -1) {
            valid = false;
            break;
        }
        const int nodeX = getX(nodeCell, nCellsSqrt);
        const int nodeY = getY(nodeCell, nCellsSqrt);
        const int nNeighIn = static_cast<int>(count(predecessors[node].begin(), predecessors[node].end(), true));
        const int nNeighOut = static_cast<int>(count(successors[node].begin(), successors[node].end(), true));
        vector<pair<int, int> > inputDir = qcaGetInputDirections(nodeX, nodeY);
        vector<pair<int, int> > outputDir = qcaGetOutputDirections(nodeX, nodeY);

        int nFound = 0;
        for (auto [dX,dY]: inputDir) {
            if (nNeighIn == 0) break;

            const int nCellX = nodeX + dX;
            const int nCellY = nodeY + dY;

            if (qcaIsInvalidCell(nCellX, nCellY, nCellsSqrt)) continue;

            const int nCell = getCellIndex(nCellX, nCellY, nCellsSqrt);

            for (int pred = 0; pred < predecessors[node].size(); pred++) {
                if (predecessors[node][pred]) {
                    const int predCell = n2c[pred];
                    if (predCell == nCell) {
                        nFound++;
                        break;
                    }
                }
            }
        }
        if (nFound != nNeighIn) {
            valid = false;
            return valid;
        }

        nFound = 0;
        for (auto [dX,dY]: outputDir) {
            if (nNeighOut == 0) break;

            const int nCellX = nodeX + dX;
            const int nCellY = nodeY + dY;

            if (qcaIsInvalidCell(nCellX, nCellY, nCellsSqrt)) continue;

            const int nCell = getCellIndex(nCellX, nCellY, nCellsSqrt);

            for (int succ = 0; succ < successors[node].size(); succ++) {
                if (successors[node][succ]) {
                    const int succCell = n2c[succ];
                    if (succCell == nCell) {
                        nFound++;
                        break;
                    }
                }
            }
        }
        if (nFound != nNeighOut) {
            valid = false;
            return valid;
        }
    }
    return valid;
}
