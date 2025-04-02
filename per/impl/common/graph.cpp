#include <common/graph.h>


/*void Graph::readGraphDataStr() {
    unordered_set<string> nodesStr;
    vector<pair<string, string> > edgesStr;

    ifstream dotFile(dotPath);
    string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << dotPath << endl;
        return;
    }

    //1 - Read edges and get a list of nodes
    while (getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != string::npos) {
            string toNode;
            string fromNode;

            istringstream iss(line);
            string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            toNode.erase(remove(toNode.begin(), toNode.end(), '\"'), toNode.end());
            fromNode.erase(remove(fromNode.begin(), fromNode.end(), '\"'), fromNode.end());
            // Add the edge to the adjacency list

            nodesStr.insert(fromNode);
            nodesStr.insert(toNode);
            edgesStr.emplace_back(fromNode, toNode);
            nEdges += 1;
        }
    }
    dotFile.close();
    nNodes = static_cast<int>(nodesStr.size());

    //2 - Create the dictionary nodesToIdx
    unordered_map<string, int> nodesToIdx;

    int counter = 0;
    for (const auto &node: nodesStr) {
        nodesToIdx[node] = counter;
        counter++;
    }

    //3 - strEdges to idxEdges
    //find the successors and the predecessors
    //and find how many succ and pred each node have

    //edgesIdx
    //successors
    //predecessors

    nSuccV = vector<int>(nNodes, 0);
    nPredV = vector<int>(nNodes, 0);
    successors = vector<vector<bool> >(nNodes, vector<bool>(nNodes, false));
    predecessors = vector<vector<bool> >(nNodes, vector<bool>(nNodes, false));
    for (const auto &[fst, snd]: edgesStr) {
        int fromN = nodesToIdx[fst], toN = nodesToIdx[snd];
        gEdges.emplace_back(fromN, toN);

        successors[fromN][toN] = true;
        predecessors[toN][fromN] = true;

        nSuccV[fromN] += 1;
        nPredV[toN] += 1;
    }

    //input and output nodes
    for (int i = 0; i < nNodes; i++) {
        if (nSuccV[i] == 0) {
            outputNodes.push_back(i);
        }
        if (nPredV[i] == 0) {
            inputNodes.push_back(i);
        }
    }

    int a = 1;

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
}*/

Graph::Graph(const string &dotPath, const string &dotName) {
    this->dotPath = dotPath;
    this->dotName = dotName;

    readEdgesNodes();
    updateG();
}

void Graph::updateG() {
    readAdjList();
    readSuccPred();
    readIONodes();
    findLongestPath();
}

void Graph::readEdgesNodes() {
    unordered_set<int> nodes;

    ifstream dotFile(dotPath);
    string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << dotPath << endl;
        return;
    }

    while (getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != string::npos) {
            string toNode;
            string fromNode;

            istringstream iss(line);
            string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            toNode.erase(remove(toNode.begin(), toNode.end(), '\"'), toNode.end());
            fromNode.erase(remove(fromNode.begin(), fromNode.end(), '\"'), fromNode.end());

            int fromN = stoi(fromNode);
            int toN = stoi(toNode);

            // Add the edge to the adjacency list

            nodes.insert(fromN);
            nodes.insert(toN);
            gEdges.emplace_back(fromN, toN);
        }
    }
    dotFile.close();
    nEdges = static_cast<int>(gEdges.size());
    nNodes = static_cast<int>(nodes.size());

    //input and output nodes
    for (int i = 0; i < nNodes; i++)
        gNodes.push_back(i);
}

void Graph::readAdjList() {
    adjList.clear();
    for (auto [fst, snd]: gEdges) {
        adjList[fst].push_back(snd);
    }
}

void Graph::readIONodes() {
    //input and output nodes
    outputNodes.clear();
    inputNodes.clear();
    otherNodes.clear();
    for (int i = 0; i < nNodes; i++) {
        if (nSuccV[i] == 0) {
            outputNodes.push_back(i);
            continue;
        }
        if (nPredV[i] == 0) {
            inputNodes.push_back(i);
            continue;
        }
        otherNodes.push_back(i);
    }
}

void Graph::readSuccPred() {
    //2 - find the successors and the predecessors
    //and find how many succ and pred each node have

    //edgesIdx
    //successors
    //predecessors
    nSuccV = vector<int>(nNodes, 0);
    nPredV = vector<int>(nNodes, 0);
    successors = vector<vector<bool> >(nNodes, vector<bool>(nNodes, false));
    predecessors = vector<vector<bool> >(nNodes, vector<bool>(nNodes, false));

    for (const auto &[fst, snd]: gEdges) {
        const int fromN = fst;
        const int toN = snd;

        successors[fromN][toN] = true;
        predecessors[toN][fromN] = true;

        nSuccV[fromN] += 1;
        nPredV[toN] += 1;
    }
}

vector<pair<int, int> > Graph::getEdgesDepthFirst() {
    // Copy input nodes and shuffle if needed
    vector<int> inputList = inputNodes;

    randomVector(inputList);

    vector<int> stack(inputList); // Initialize stack with input_list
    vector<pair<int, int> > edges;
    vector<bool> visited(nNodes, false);

    while (!stack.empty()) {
        int n = stack.back();
        stack.pop_back();


        if (visited[n]) {
            continue;
        }
        visited[n] = true;
        bool flag = false;
        // Process all neighbors
        for (int i = 0; i < nNodes; i++) {
            if (successors[n][i]) {
                if (!visited[i]) {
                    stack.push_back(i);
                    edges.emplace_back(n, i);
                    flag = true;
                }
            }
        }
    }

    return edges;
}

vector<pair<int, int> > Graph::getEdgesDepthFirstPriority() {
    // Copia os nós de entrada e embaralha, se necessário
    vector<int> inputList = inputNodes;
    randomVector(inputList);
    //move the initial node of the longest path to the end of the stack and so on

    auto it = std::find(inputList.begin(), inputList.end(), longestPath[0]);

    if (it != inputList.end()) {
        const int valor = *it;
        inputList.erase(it); // remove node
        inputList.push_back(valor); // add it on last position
    }

    // Inicializa a pilha com inputList
    vector<int> stack(inputList);
    vector<pair<int, int> > edges;
    vector<bool> visited(nNodes, false);

    while (!stack.empty()) {
        int n = stack.back();
        stack.pop_back();

        if (visited[n]) {
            continue;
        }
        visited[n] = true;

        // Coleta os vizinhos ainda não visitados
        vector<int> neighbors;
        for (int i = 0; i < nNodes; i++) {
            if (successors[n][i] && !visited[i]) {
                neighbors.push_back(i);
            }
        }

        // Ordena os vizinhos para priorizar os maiores caminhos
        sort(neighbors.begin(), neighbors.end(), [this](int a, int b) {
            // Critério para ordenar: pode ser baseado na quantidade de sucessores
            return count(successors[a].begin(), successors[a].end(), true) >
                   count(successors[b].begin(), successors[b].end(), true);
        });

        // Adiciona os vizinhos à pilha e armazena as arestas
        for (int neighbor: neighbors) {
            stack.push_back(neighbor);
            edges.emplace_back(n, neighbor);
        }
    }

    return edges;
}

vector<pair<int, int> > Graph::getEdgesZigzag(vector<pair<int, int> > &convergence) {
    vector<pair<int, string> > outputList;

    for (const auto &node: outputNodes) {
        outputList.emplace_back(node, "IN");
    }

    // if (make_shuffle) {
    randomVector(outputList);
    // }

    vector stack(outputList.begin(), outputList.end());
    vector<pair<int, int> > edges;
    vector visited(nNodes, false);

    // Precompute fan-in and fan-out
    vector<vector<int> > fanIn(nNodes);
    vector<vector<int> > fanOut(nNodes);
    for (int i = 0; i < nNodes; i++) {
        for (int j = 0; j < nNodes; j++) {
            if (successors[i][j]) {
                fanOut[i].push_back(j);
            }
            if (predecessors[i][j]) {
                fanIn[i].push_back(j);
            }
        }
        randomVector(fanOut[i]);
        randomVector(fanIn[i]);
    }


    while (!stack.empty()) {
        auto [fst, snd] = stack.back();
        stack.pop_back();

        const int &a = fst;
        const string &direction = snd;
        visited[a] = true;;

        if (direction == "IN") {
            if (!fanOut[a].empty()) {
                const int b = fanOut[a].back();
                stack.emplace_back(a, "IN");
                stack.insert(stack.end(), fanIn[a].size(), {a, "IN"});
                stack.emplace_back(b, "OUT");

                fanOut[a].pop_back();
                fanIn[b].erase(remove(fanIn[b].begin(), fanIn[b].end(), a), fanIn[b].end());

                if (visited[b]) {
                    convergence.emplace_back(a, b);
                }
                //edges.emplace_back({a, b, "OUT"});
                edges.emplace_back(a, b);
            } else if (!fanIn[a].empty()) {
                const int b = fanIn[a].back();
                stack.emplace_back(a, "IN");
                stack.insert(stack.end(), fanIn[a].size(), {b, "IN"});

                fanIn[a].pop_back();
                fanOut[b].erase(remove(fanOut[b].begin(), fanOut[b].end(), a), fanOut[b].end());


                if (visited[b]) {
                    convergence.emplace_back(a, b);
                }
                // edges.push_back({a, b, "IN"});
                edges.emplace_back(a, b);
            }
        } else {
            // direction == "OUT"
            if (!fanIn[a].empty()) {
                int b = fanIn[a].back();
                stack.emplace_back(a, "OUT");
                stack.insert(stack.end(), fanOut[a].size(), {a, "OUT"});
                stack.emplace_back(b, "IN");

                fanIn[a].pop_back();
                fanOut[b].erase(remove(fanOut[b].begin(), fanOut[b].end(), a), fanOut[b].end());

                if (visited[b]) {
                    convergence.emplace_back(a, b);
                }
                //edges.push_back({a, b, "IN"});
                edges.emplace_back(a, b);
            } else if (!fanOut[a].empty()) {
                int b = fanOut[a].back();
                stack.emplace_back(a, "OUT");
                stack.insert(stack.end(), fanOut[a].size(), {b, "OUT"});

                fanOut[a].pop_back();
                fanIn[b].erase(remove(fanIn[b].begin(), fanIn[b].end(), a), fanIn[b].end());


                if (visited[b]) {
                    convergence.emplace_back(a, b);
                }
                // edges.push_back({a, b, "OUT"});
                edges.emplace_back(a, b);
            }
        }
    }
    return clearEdges(edges);
}

vector<pair<int, int> > Graph::clearEdges(const vector<pair<int, int> > &edges) const {
    vector placedNodes(nNodes, false); // Set to track placed nodes
    vector<pair<int, int> > new_edges; // Vector to store filtered edges

    // Add the first node of the first edge to the set
    placedNodes[edges[0].first] = true;

    for (const auto [fst,snd]: edges) {
        // Check if the second node is not in the set or if we don't remove placed edges
        if (!placedNodes[snd]) {
            placedNodes[snd] = true; // Add n2 to the set
            new_edges.emplace_back(fst, snd); // Add the edge to the new list
        }
    }
    return new_edges;
}

void Graph::findLongestPath() {
    // 1. Construir lista de adjacência a partir da matriz de sucessores
    vector<vector<int> > adj(nNodes);
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

    // 3. Programação dinâmica para encontrar o maior caminho
    vector<int> dist(nNodes, numeric_limits<int>::min());
    vector<int> parent(nNodes, -1);
    for (int i = 0; i < nNodes; ++i)
        dist[i] = 0; // cada nó pode iniciar um caminho de comprimento 0

    for (const int u: topo_order) {
        for (const int v: adj[u]) {
            if (dist[u] + 1 > dist[v]) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
            }
        }
    }

    // 4. Encontrar o nó final do maior caminho
    int max_len = -1, end_node = -1;
    for (int i = 0; i < nNodes; ++i) {
        if (dist[i] > max_len) {
            max_len = dist[i];
            end_node = i;
        }
    }

    // 5. Reconstruir o caminho
    vector<int> path;
    for (int v = end_node; v != -1; v = parent[v])
        path.push_back(v);
    reverse(path.begin(), path.end());

    longestPath = path;
}

void Graph::dfs(const int idx, const vector<vector<int> > &adj, vector<bool> &visited, vector<int> &topo_order) {
    visited[idx] = true;
    for (const int v: adj[idx]) {
        if (!visited[v]) {
            dfs(v, adj, visited, topo_order);
        }
    }
    topo_order.push_back(idx);
}
