#include "graph.h"


void getGraphData() {
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
            nEdges++;

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
    successors = std::vector<bool>(nNodes * nNodes, false);
    predecessors = std::vector<bool>(nNodes * nNodes, false);
    for (const auto &[fst, snd]: edgesStr) {
        int nodeF = nodesToIdx[fst], nodeT = nodesToIdx[snd];
        gEdges.emplace_back(nodeF, nodeT);

        int l_offset;
        l_offset = nodeF * nNodes;
        successors[l_offset + nodeT] = true;
        l_offset = nodeT * nNodes;
        predecessors[l_offset + nodeF] = true;

        nSuccV[nodeF] += 1;
        nPredV[nodeT] += 1;
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

//FIXME
std::vector<std::pair<int, int> > getEdgesDepthFirst() {
    // Copy input nodes and shuffle if needed
    std::vector<std::string> input_list = inputNodesStr;

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(input_list.begin(), input_list.end(), g);

    std::vector<std::string> stack(input_list); // Initialize stack with input_list
    std::vector<std::pair<std::string, std::string> > edges;
    std::unordered_set<std::string> visited;

    while (!stack.empty()) {
        std::string n = stack.back();
        stack.pop_back();

        if (visited.find(n) != visited.end()) {
            continue;
        }
        visited.insert(n);


        // Process all neighbors without priority
        for (const auto &neigh: succ[n]) {
            if (visited.find(neigh) == visited.end()) {
                stack.push_back(neigh);
                edges.emplace_back(n, neigh);
            }
        }
    }

    return edges;
}