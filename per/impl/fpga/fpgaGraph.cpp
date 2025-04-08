#include <fpga/fpgaGraph.h>
#include <algorithm>
#include <list>


FPGAGraph::FPGAGraph(const string& dotPath, const string& dotName): Graph(dotPath, dotName)
{
    readNeighbors();
    calcMatrix();
    clbNodes = otherNodes;
}

void FPGAGraph::updateG()
{
    Graph::updateG();
    findLongestPath();
}

void FPGAGraph::readNeighbors()
{
    //neighbors vector
    neighbors.resize(nNodes);
    for (size_t i = 0; i < successors.size(); ++i)
    {
        for (size_t j = 0; j < successors[i].size(); ++j)
        {
            if (successors[i][j] || predecessors[i][j])
                neighbors[i].push_back(static_cast<int>(j));
        }
    }
}

void FPGAGraph::calcMatrix()
{
    const int totalInOut = static_cast<int>(inputNodes.size() + outputNodes.size());
    const int nBaseNodes = nNodes - totalInOut;
    int nCellsBaseSqrt = ceil(sqrt(nBaseNodes));
    int nBorderCells = nCellsBaseSqrt * 4;
    while (totalInOut > nBorderCells)
    {
        nCellsBaseSqrt += 2;
        nBorderCells = nCellsBaseSqrt * 4;
    }
    const int nCellsBase = static_cast<int>(pow(nCellsBaseSqrt, 2));
    const int totalCells = nCellsBase + nBorderCells;
    nCellsSqrt = ceil(sqrt(totalCells));
    nCells = static_cast<int>(pow(nCellsSqrt, 2));
}

vector<int> FPGAGraph::getInOutPos() const
{
    vector<int> possibleInOut;

    // Append positions in the first range
    for (int i = 1; i < nCellsSqrt - 1; ++i)
        possibleInOut.push_back(i);

    // Append positions in the second range
    for (int i = 1; i < nCellsSqrt - 1; ++i)
        possibleInOut.push_back(i + nCells - nCellsSqrt);

    // Append positions in the third range
    for (int i = nCellsSqrt; i < nCells - nCellsSqrt; i += nCellsSqrt)
        possibleInOut.push_back(i);

    // Append positions in the fourth range
    for (int i = nCellsSqrt * 2 - 1; i < nCells - 1; i += nCellsSqrt)
        possibleInOut.push_back(i);

    return possibleInOut;
}

vector<int> FPGAGraph::getClbPos() const
{
    vector<int> pos;
    for (int i = 1; i < nCellsSqrt - 1; i++)
    {
        const int start = i * nCellsSqrt + 1;
        const int end = (i + 1) * nCellsSqrt - 1;
        for (int j = start; j < end; j++)
            pos.push_back(j);
    }
    return pos;
}

unordered_map<string, vector<pair<int, int>>> FPGAGraph::getGraphAnnotations(
    const vector<pair<int, int>>& edges,
    const vector<pair<int, int>>& convergences
)
{
    unordered_map<string, vector<pair<int, int>>> annotations;
    vector<int> placed;
    // Initialization of the dictionary
    placed.push_back(edges[0].first);
    for (const auto& [fst, snd] : edges)
    {
        string key = funcKey(to_string(fst), to_string(snd));
        annotations[key] = {};
        placed.push_back(snd);

        //This code tries to make yott guide the neighbot IO nodes to be near the border
        //It is an improvement to the yott code
        bool ioNode = false;
        for (const auto& node : neighbors[snd])
        {
            const bool tmp = find(inputNodes.begin(), inputNodes.end(), node) != inputNodes.end() ||
                find(outputNodes.begin(), outputNodes.end(), node) != outputNodes.end();
            if (tmp)
            {
                const bool tmp2 = find(placed.begin(), placed.end(), node) != placed.end();
                if (!tmp2)
                    ioNode = true;
            }
        }
        if (ioNode)
            annotations[key].emplace_back(-1, -1);
    }

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

void FPGAGraph::findLongestPath()
{
    // 1. Construct adjacency list from successor array
    vector<vector<int>> adj(nNodes);
    for (int i = 0; i < nNodes; ++i)
        for (int j = 0; j < nNodes; ++j)
            if (successors[i][j])
                adj[i].push_back(j);

    // 2. Ordenação topológica
    vector<bool> visited(nNodes, false);
    vector<int> topo_order;
    for (int i = 0; i < nNodes; ++i)
        if (!visited[i])
            dfs(i, adj, visited, topo_order);
    reverse(topo_order.begin(), topo_order.end());

    // 3. Dynamic programming to find the longest path
    //vector dist(nNodes, numeric_limits<int>::min());
    vector dist(nNodes, 0);
    vector parent(nNodes, -1);

    /*for (int i = 0; i < nNodes; ++i)
        dist[i] = 0; // each node can start a path of length 0
        */

    for (const int u : topo_order)
    {
        for (const int v : adj[u])
        {
            if (dist[u] + 1 > dist[v])
            {
                dist[v] = dist[u] + 1;
                parent[v] = u;
            }
        }
    }

    // 4. Find the end node of the longest path
    int max_len = -1, end_node = -1;
    for (int i = 0; i < nNodes; ++i)
    {
        if (dist[i] > max_len)
        {
            max_len = dist[i];
            end_node = i;
        }
    }

    // 5. Rebuild the path
    vector<int> path;
    for (int v = end_node; v != -1; v = parent[v])
        path.push_back(v);
    reverse(path.begin(), path.end());

    longestPath = path;
}


vector<pair<int, int>> FPGAGraph::getEdgesDepthFirstPriority()
{
    // Copia os nós de entrada e embaralha, se necessário
    vector<int> inputList = inputNodes;
    randomVector(inputList);
    //move the initial node of the longest path to the end of the stack and so on
    const auto it = std::find(inputList.begin(), inputList.end(), longestPath[0]);

    if (it != inputList.end())
    {
        const int valor = *it;
        inputList.erase(it); // remove node
        inputList.push_back(valor); // add it on last position
    }

    // Inicializa a pilha com inputList
    vector<int> stack(inputList);
    vector<pair<int, int>> edges;
    vector<bool> visited(nNodes, false);

    while (!stack.empty())
    {
        int n = stack.back();
        stack.pop_back();

        if (visited[n])
            continue;

        visited[n] = true;

        // Coleta os vizinhos ainda não visitados
        vector<int> neighbors;
        for (int i = 0; i < nNodes; i++)
        {
            if (successors[n][i] && !visited[i])
                neighbors.push_back(i);

        }

        // Ordena os vizinhos para priorizar os maiores caminhos
        sort(neighbors.begin(), neighbors.end(), [this](int a, int b)
        {
            // Critério para ordenar: pode ser baseado na quantidade de sucessores
            return count(successors[a].begin(), successors[a].end(), true) >
                count(successors[b].begin(), successors[b].end(), true);
        });

        // Adiciona os vizinhos à pilha e armazena as arestas
        for (int neighbor : neighbors)
        {
            stack.push_back(neighbor);
            edges.emplace_back(n, neighbor);
        }
    }

    return edges;
}
