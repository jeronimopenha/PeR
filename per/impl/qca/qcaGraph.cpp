#include <algorithm>
#include <queue>
#include <qca/qcaGraph.h>
#include<common/parameters.h>


QCAGraph::QCAGraph(const string& dotPath, const string& dotName): Graph(dotPath, dotName)
{
    exportUpGToDot("/home/jeronimo/qca.dot");
    fixFanout();
    fixFanin();
    balanceGraphAll();
}

void QCAGraph::calcMatrix()
{
    nCellsSqrt = static_cast<int>(ceil(sqrt(nNodes))) * 2;
    nCells = static_cast<int>(pow(nCellsSqrt, 2));
}

void QCAGraph::fixFanout()
{
    unordered_map<int, vector<int>> fanouts;
    for (auto [fst, snd] : gEdges)
    {
        fanouts[fst].push_back(snd);
    }

    vector<pair<int, int>> newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto& [fst, outs] : fanouts)
    {
        if (outs.size() <= 2)
        {
            for (int snd : outs) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<int> leaves;
        for (int snd : outs) leaves.push(snd);

        vector<int> dummyNodes;

        while (leaves.size() > 1)
        {
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

#ifdef DEBUG
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

void QCAGraph::fixFanin()
{
    unordered_map<int, vector<int>> fanins;
    for (auto [fst, snd] : gEdges)
    {
        fanins[snd].push_back(fst);
    }

    vector<pair<int, int>> newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto& [snd, ins] : fanins)
    {
        if (ins.size() <= 2)
        {
            for (int fst : ins) newEdges.emplace_back(fst, snd);
            continue;
        }

        queue<int> leaves;
        for (int fst : ins) leaves.push(fst);

        while (leaves.size() > 1)
        {
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
#ifdef DEBUG
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

void QCAGraph::computeLevels()
{
    level.clear();
    levelSuccessors.clear();
    levelPredecessors.clear();

    unordered_map<int, int> inDegree;

    for (auto [fst, snd] : gEdges)
    {
        inDegree[snd]++;
    }

    queue<int> q;
    for (int node : inputNodes)
    {
        q.push(node);
        level[node] = 0;
    }

    while (!q.empty())
    {
        int fst = q.front();
        q.pop();
        for (int snd : adjList[fst])
        {
            level[snd] = max(level[snd], level[fst] + 1);
            if (--inDegree[snd] == 0)
                q.push(snd);
        }
    }

    for (auto [fst, snd] : gEdges)
    {
        levelSuccessors[level[fst]].push_back(snd);
        levelPredecessors[level[snd]].push_back(fst);
    }
    minOutputLevel = numeric_limits<int>::max();
    for (int out : outputNodes)
    {
        if (level.count(out))
        {
            minOutputLevel = min(minOutputLevel, level[out]);
        }
    }
    calcMatrix();
}

// Balance entire graph by inserting dummy nodes
void QCAGraph::balanceGraphAll()
{
    computeLevels();

    vector<pair<int, int>> newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [fst, snd] : gEdges)
    {
        const int lfst = level[fst];
        int lsnd = level[snd];

        if (lsnd == lfst + 1)
        {
            newEdges.emplace_back(fst, snd);
        }
        else if (lsnd > lfst + 1)
        {
            int last = fst;
            for (int i = 0; i < lsnd - lfst - 1; ++i)
            {
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

#ifdef DEBUG
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

// Insert dummy layer between nodes at level L and L+1
void QCAGraph::insertDummyLayerAtLevel(const int targetLevel)
{
    computeLevels();

    vector<pair<int, int>> newEdges;
    int nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;
    int dummyCount = 0;

    for (auto [fst, snd] : gEdges)
    {
        const int lfst = level[fst];
        int lsnd = level[snd];

        if (lfst == targetLevel && lsnd == targetLevel + 1)
        {
            int dummy = nextId++;
            gNodes.push_back(dummy);
            dummyMap[dummy] = "dummy" + to_string(dummyCount++);
            newEdges.emplace_back(fst, dummy);
            newEdges.emplace_back(dummy, snd);
        }
        else
        {
            newEdges.emplace_back(fst, snd);
        }
    }

    gEdges = newEdges;
    nEdges = static_cast<int>(gEdges.size());
    nNodes = nextId;

    updateG();
    computeLevels();

#ifdef DEBUG
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

// Export graph as DOT format
void QCAGraph::exportUpGToDot(const string& filename)
{
    ofstream fout(filename);
    fout << "digraph network {\n";
    for (auto [fst, snd] : gEdges)
    {
        fout << "  " << fst << " -> " << snd;
        if (dummyMap.count(fst) || dummyMap.count(snd))
        {
            fout << " [color=gray, style=dashed]";
        }
        fout << ";\n";
    }
    fout << "}\n";
    fout.close();
    //cout << "DOT file exported to: " << filename << endl;
}

// Save dummy node map
void QCAGraph::saveDummyMap(const string& filename)
{
    ofstream fout(filename);
    for (auto& [id, name] : dummyMap)
    {
        fout << id << " " << name << "\n";
    }
    fout.close();
    cout << "Dummy map saved to: " << filename << endl;
}
