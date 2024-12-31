#include "graph.h"

#include <util.h>

void graphClearData() {
    nEdges = 0;
    nNodes = 0;
    nCells = 0;
    nCellsSqrt = 0;

    successors.clear();
    //Adjacency for predecessors
    predecessors.clear();
    //Edges list
    gEdges.clear();
    //input nodes
    nSuccV.clear();
    //output nodes
    nPredV.clear();
    inputNodes.clear();
    outputNodes.clear();
}


void getGraphDataStr() {
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

void getGraphDataInt() {
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


std::vector<std::pair<int, int> > getEdgesDepthFirst() {
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
        if(!flag) {
            int a = 1;
        }
    }

    return edges;
}

std::vector<int> getInOutPos() {
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
