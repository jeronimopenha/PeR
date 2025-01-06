#include "graph.h"
#include <util.h>

Graph::Graph(const std::string &dotPath, const std::string &dotName) {
    this->dotPath = dotPath;
    this->dotName = dotName;
}

void Graph::getGraphDataStr() {
    std::unordered_set<std::string> nodesStr;
    std::vector<std::pair<std::string, std::string> > edgesStr;

    std::ifstream dotFile(dotPath);
    std::string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        std::cerr << "Error opening file: " << dotPath << std::endl;
        return;
    }

    //1 - Read edges and get a list of nodes
    while (std::getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != std::string::npos) {
            std::string toNode;
            std::string fromNode;

            std::istringstream iss(line);
            std::string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(std::remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            toNode.erase(std::remove(toNode.begin(), toNode.end(), '\"'), toNode.end());
            fromNode.erase(std::remove(fromNode.begin(), fromNode.end(), '\"'), fromNode.end());
            // Add the edge to the adjacency list

            nodesStr.insert(fromNode);
            nodesStr.insert(toNode);
            edgesStr.emplace_back(fromNode, toNode);
            nEdges += 1;
        }
    }
    dotFile.close();
    nNodes = static_cast<int>(nodesStr.size());

    //2 - Create the dictinary nodesToIdx
    std::unordered_map<std::string, int> nodesToIdx;

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

    nSuccV = std::vector<int>(nNodes, 0);
    nPredV = std::vector<int>(nNodes, 0);
    successors = std::vector<std::vector<bool> >(nNodes, std::vector<bool>(nNodes, false));
    predecessors = std::vector<std::vector<bool> >(nNodes, std::vector<bool>(nNodes, false));
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
}

void Graph::getGraphDataInt() {
    std::unordered_set<int> nodes;

    std::ifstream dotFile(dotPath);
    std::string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        std::cerr << "Error opening file: " << dotPath << std::endl;
        return;
    }

    //1 - Read edges and get a list of nodes
    while (std::getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != std::string::npos) {
            std::string toNode;
            std::string fromNode;

            std::istringstream iss(line);
            std::string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(std::remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            toNode.erase(std::remove(toNode.begin(), toNode.end(), '\"'), toNode.end());
            fromNode.erase(std::remove(fromNode.begin(), fromNode.end(), '\"'), fromNode.end());

            int fromN = std::stoi(fromNode);
            int toN = std::stoi(toNode);

            // Add the edge to the adjacency list

            nodes.insert(fromN);
            nodes.insert(toN);
            gEdges.emplace_back(fromN, toN);
        }
    }
    dotFile.close();
    nEdges = static_cast<int>(gEdges.size());
    nNodes = static_cast<int>(nodes.size());

    //2 - find the successors and the predecessors
    //and find how many succ and pred each node have

    //edgesIdx
    //successors
    //predecessors

    nSuccV = std::vector<int>(nNodes, 0);
    nPredV = std::vector<int>(nNodes, 0);
    successors = std::vector<std::vector<bool> >(nNodes, std::vector<bool>(nNodes, false));
    predecessors = std::vector<std::vector<bool> >(nNodes, std::vector<bool>(nNodes, false));
    for (const auto &[fst, snd]: gEdges) {
        int fromN = fst, toN = snd;

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

std::vector<int> Graph::getInOutPos() {
    std::vector<int> possibleInOut;

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

std::vector<std::pair<int, int> > Graph::getEdgesDepthFirst() {
    // Copy input nodes and shuffle if needed
    std::vector<int> inputList = inputNodes;

    randomVector(inputList);

    std::vector<int> stack(inputList); // Initialize stack with input_list
    std::vector<std::pair<int, int> > edges;
    std::vector<bool> visited(nNodes, false);

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
        if (!flag) {
            int a = 1;
        }
    }

    return edges;
}

std::vector<std::pair<int, int> > Graph::getEdgesDepthFirstPriority() {
    // Copia os nós de entrada e embaralha, se necessário
    std::vector<int> inputList = inputNodes;
    randomVector(inputList);

    // Inicializa a pilha com inputList
    std::vector<int> stack(inputList);
    std::vector<std::pair<int, int> > edges;
    std::vector<bool> visited(nNodes, false);

    while (!stack.empty()) {
        int n = stack.back();
        stack.pop_back();

        if (visited[n]) {
            continue;
        }
        visited[n] = true;

        // Coleta os vizinhos ainda não visitados
        std::vector<int> neighbors;
        for (int i = 0; i < nNodes; i++) {
            if (successors[n][i] && !visited[i]) {
                neighbors.push_back(i);
            }
        }

        // Ordena os vizinhos para priorizar os maiores caminhos
        std::sort(neighbors.begin(), neighbors.end(), [this](int a, int b) {
            // Critério para ordenar: pode ser baseado na quantidade de sucessores
            return std::count(successors[a].begin(), successors[a].end(), true) >
                   std::count(successors[b].begin(), successors[b].end(), true);
        });

        // Adiciona os vizinhos à pilha e armazena as arestas
        for (int neighbor: neighbors) {
            stack.push_back(neighbor);
            edges.emplace_back(n, neighbor);
        }
    }

    return edges;
}

std::vector<std::pair<int, int> > Graph::getEdgesZigzag() {
    std::vector<std::pair<int, std::string> > outputList;

    for (const auto &node: outputNodes) {
        outputList.emplace_back(node, "IN");
    }

    // if (make_shuffle) {
    randomVector(outputList);
    // }

    std::vector<std::pair<int, std::string> > stack(outputList.begin(), outputList.end());
    std::vector<std::pair<int, int> > edges;
    std::vector<bool> visited(nNodes, false);
    //std::vector<std::vector<std::string> > convergence;

    // Precompute fan-in and fan-out
    std::vector<std::vector<int> > fanIn(nNodes);
    std::vector<std::vector<int> > fanOut(nNodes);
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

    // if (make_shuffle) {
    //     for (auto &[node, neighbors]: fan_in) shuffleVector(neighbors);
    //     for (auto &[node, neighbors]: fan_out) shuffleVector(neighbors);
    // }

    while (!stack.empty()) {
        auto [fst, snd] = stack.back();
        stack.pop_back();

        const int &a = fst;
        const std::string &direction = snd;
        visited[a] = true;;

        if (direction == "IN") {
            if (!fanOut[a].empty()) {
                const int b = fanOut[a].back();
                stack.emplace_back(a, "IN");
                stack.insert(stack.end(), fanIn[a].size(), {a, "IN"});
                stack.emplace_back(b, "OUT");

                fanOut[a].pop_back();
                fanIn[b].erase(std::remove(fanIn[b].begin(), fanIn[b].end(), a), fanIn[b].end());

                //if (visited.count(b)) {
                //     convergence.push_back({a, b});
                // }
                //edges.emplace_back({a, b, "OUT"});
                edges.emplace_back(a, b);
            } else if (!fanIn[a].empty()) {
                const int b = fanIn[a].back();
                stack.emplace_back(a, "IN");
                stack.insert(stack.end(), fanIn[a].size(), {b, "IN"});

                fanIn[a].pop_back();
                fanOut[b].erase(std::remove(fanOut[b].begin(), fanOut[b].end(), a), fanOut[b].end());

                //
                // if (visited.count(b)) {
                //     convergence.push_back({a, b});
                // }
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
                fanOut[b].erase(std::remove(fanOut[b].begin(), fanOut[b].end(), a), fanOut[b].end());

                // if (visited.count(b)) {
                //     convergence.push_back({a, b});
                // }
                //edges.push_back({a, b, "IN"});
                edges.emplace_back(a, b);
            } else if (!fanOut[a].empty()) {
                int b = fanOut[a].back();
                stack.emplace_back(a, "OUT");
                stack.insert(stack.end(), fanOut[a].size(), {b, "OUT"});

                fanOut[a].pop_back();
                fanIn[b].erase(std::remove(fanIn[b].begin(), fanIn[b].end(), a), fanIn[b].end());


                // if (visited.count(b)) {
                //     convergence.push_back({a, b});
                // }
                // edges.push_back({a, b, "OUT"});
                edges.emplace_back(a, b);
            }
        }
    }
    return clearEdges(edges);
}


std::vector<std::pair<int, int> > Graph::clearEdges(const std::vector<std::pair<int, int> > &edges) {
    std::vector placedNodes(nNodes, false); // Set to track placed nodes
    std::vector<std::pair<int, int> > new_edges; // Vector to store filtered edges

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
