#include <algorithm>
#include <queue>
#include <qca/qcaGraph.h>


QCAGraph::QCAGraph(const string &dotPath, const string &dotName): Graph(dotPath, dotName) {
    balanceGraphAll();
    insertDummyLayerAtLevel(3);
    insertDummyLayerAtLevel(1);
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
    for (int out : outputNodes) {
        if (level.count(out)) {
            minOutputLevel = min(minOutputLevel, level[out]);
        }
    }
}

// Balance entire graph by inserting dummy nodes
void QCAGraph::balanceGraphAll() {
    computeLevels();

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [fst, snd]: gEdges) {
        int lfst = level[fst];
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

    //exportUpGToDot("/home/jeronimo/qca.dot");

    updateG();
    computeLevels();
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
    cout << "DOT file exported to: " << filename << endl;
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

// Insert dummy layer between nodes at level L and L+1
void QCAGraph::insertDummyLayerAtLevel(const int targetLevel) {
    computeLevels();

    vector<pair<int, int> > newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [fst, snd]: gEdges) {
        int lfst = level[fst];
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

    //exportUpGToDot("/home/jeronimo/qca.dot");
    updateG();
    computeLevels();
}
