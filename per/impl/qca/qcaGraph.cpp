#include <algorithm>
#include <list>
#include <queue>
#include <qca/qcaGraph.h>
#include<common/parameters.h>
#include <qca/qcaUtil.h>


QCAGraph::QCAGraph(const string& dotPath, const string& dotName): Graph(dotPath, dotName)
{
#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
    fixFanout();
    fixFanin();
    balanceGraphAll();
}

void QCAGraph::updateG()
{
    Graph::updateG();
    outNeighbors = std::vector<std::vector<int>>(nNodes);
    inNeighbors = std::vector<std::vector<int>>(nNodes);
    updateNeighbors();
}

void QCAGraph::updateNeighbors()
{
    for (int node = 0; node < nNodes; node++)
    {
        for (int neigh = 0; neigh < nNodes; neigh++)
        {
            if (successors[node][neigh])
                outNeighbors[node].push_back(neigh);

            if (predecessors[node][neigh])
                inNeighbors[node].push_back(neigh);
        }
    }
}

void QCAGraph::calcMatrix()
{
    nCellsSqrt = static_cast<int>(ceil(sqrt(nNodes))) + 2;
    nCells = static_cast<int>(pow(nCellsSqrt, 2));
}

void QCAGraph::fixFanout()
{
    unordered_map<int, vector<int>> fanouts;
    for (auto [fst, snd] : gEdges)
        fanouts[fst].push_back(snd);


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
        for (int snd : outs)
            leaves.push(snd);

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

#ifdef PRINT
    exportUpGToDot("/home/jeronimo/qca.dot");
#endif
}

void QCAGraph::fixFanin()
{
    unordered_map<int, vector<int>> fanins;
    for (auto [fst, snd] : gEdges)
        fanins[snd].push_back(fst);


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
        for (int fst : ins)
            leaves.push(fst);

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
#ifdef PRINT
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
        inDegree[snd]++;


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
            newEdges.emplace_back(fst, snd);
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

#ifdef PRINT
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
            newEdges.emplace_back(fst, snd);
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
void QCAGraph::exportUpGToDot(const string& filename)
{
    ofstream fout(filename);
    fout << "digraph network {\n";

    // Declara os nós com estilo
    for (int node : gNodes)
    {
        fout << "  " << node;
        if (dummyMap.count(node))
            fout << " [style=dashed]";

        fout << ";\n";
    }

    // Escreve as arestas normalmente
    for (auto [fst, snd] : gEdges)
        fout << "  " << fst << " -> " << snd << ";\n";

    fout << "}\n";
    fout.close();
    //cout << "DOT file exported to: " << filename << endl;
}

// Save dummy node map
void QCAGraph::saveDummyMap(const string& filename)
{
    ofstream fout(filename);
    for (auto& [id, name] : dummyMap)
        fout << id << " " << name << "\n";

    fout.close();
    cout << "Dummy map saved to: " << filename << endl;
}

bool QCAGraph::verifyPlacement(const vector<int>& n2c, const vector<pair<int, int>>& edges,
                               int* invalidEdgesCount) const
{
    bool valid = true;
    int totalInvalid = 0;

    for (const auto& [src, dst] : edges)
    {
        const int srcCell = n2c[src];
        const int dstCell = n2c[dst];

        if (srcCell == -1 || dstCell == -1)
        {
            ++totalInvalid;
            valid = false;
            continue;
        }

        const int srcX = getX(srcCell, nCellsSqrt);
        const int srcY = getY(srcCell, nCellsSqrt);
        const int dstX = getX(dstCell, nCellsSqrt);
        const int dstY = getY(dstCell, nCellsSqrt);

        vector<pair<int, int>> outputDirs = qcaGetOutputDirections(srcX, srcY);

        bool found = false;
        for (auto [dx, dy] : outputDirs)
        {
            if (dstX == srcX + dx && dstY == srcY + dy)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            ++totalInvalid;
            valid = false;
        }
    }

    if (invalidEdgesCount != nullptr)
        *invalidEdgesCount = totalInvalid;

    return valid;
}

unordered_map<string, vector<pair<int, int>>> QCAGraph::qcaGetGraphAnnotations(
    const vector<pair<int, int>>& edges,
    const vector<pair<int, int>>& convergences
)
{
    unordered_map<string, vector<pair<int, int>>> annotations;
    vector<int> placed;
    // Initialization of the dictionary

    for (const auto& [fst,snd] : convergences)
    {
        const int elem_cycle_begin = fst;
        int elem_cycle_end = snd;
        list<string> walk_key;
        bool found_start = false;
        int count = 0;
        int value1 = -1;

        for (auto it = edges.rbegin(); it != edges.rend(); ++it)
        {
            const int a = it->first;

            if (const int b = it->second; elem_cycle_begin == b && !found_start)
            {
                value1 = a;
                string key = funcKey(to_string(value1), to_string(elem_cycle_begin));
                walk_key.push_front(key);
                annotations[key].emplace_back(elem_cycle_end, count);
                count++;
                found_start = true;
            }
            else if (found_start && (value1 == b || elem_cycle_end == a))
            {
                const int value2 = b;
                value1 = a;
                string key = funcKey(to_string(value1), to_string(value2));

                if (value1 != elem_cycle_end && value2 != elem_cycle_end)
                {
                    walk_key.push_front(key);
                    annotations[key].emplace_back(elem_cycle_end, count);
                    count++;
                }
                else
                {
                    // Go back and update values
                    const int half_count = count / 2;
                    auto it2 = walk_key.begin();
                    for (int k = 0; k < half_count; ++k, ++it2)
                    {
                        auto& dic_actual = annotations[*it2];
                        for (auto& [fst,snd] : dic_actual)
                        {
                            if (fst == elem_cycle_end)
                                snd = k + 1;
                        }
                    }
                    break; // Move to the next element in convergences
                }
            }
        }
    }

    return annotations;
}