#include "graph.h"


void getGraphData() {
    std::unordered_map<std::string, std::unordered_set<std::string> > succ_str;
    std::unordered_map<std::string, std::unordered_set<std::string> > pred_str;
    std::unordered_set<std::string> nodesStr;
    std::unordered_map<std::string, int> nodesToIdx;
    std::unordered_map<int, std::string> idxToNodes;

    std::vector<std::pair<std::string, std::string> > edgesStr;
    std::vector<std::string> inputNodesStr;
    std::vector<std::string> outputNodesStr;

    std::ifstream dotFile(dotPath);
    std::string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        std::cerr << "Error opening file: " << dotPath << std::endl;
        return;
    }

    //FIXME inplement the idx references here
    //Here I will get the nodes and edges to use in other parts of the code
    while (std::getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != std::string::npos) {
            std::string toNode;
            std::string fromNode;
            n_edges++;

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
            succ_str[fromNode].insert(toNode);
            pred_str[toNode].insert(fromNode);
            edgesStr.emplace_back(fromNode, toNode);
        }
    }
    dotFile.close();
    nNodes = static_cast<int>(nodesStr.size());

    //nodestoidx and idxtonodes
    //inputNodes str and idx
    //outputNodes str and idx
    int counter = 0;
    for (auto node: nodesStr) {
        nodesToIdx[node] = counter;
        idxToNodes[counter] = node;

        if (auto it = succ_str.find(node); it == succ_str.end()) {
            outputNodesStr.push_back(node);
            outputNodes.push_back(counter);
        }
        if (auto it = pred_str.find(node); it == pred_str.end()) {
            inputNodesStr.push_back(node);
            inputNodes.push_back(counter);
        }
        counter++;
    }

    const int adjacencySize = static_cast<int>(pow(2, ceil(log2(static_cast<int>(nNodes)))));

    successors = std::vector<std::vector<int> >(adjacencySize, std::vector<int>(adjacencySize, -1));
    predecessors = std::vector<std::vector<int> >(adjacencySize, std::vector<int>(adjacencySize, -1));

    //edgesIdx
    for (const auto &[fst, snd]: edgesStr) {
        edges.emplace_back(nodesToIdx[fst], nodesToIdx[snd]);
    }

    int totalInOut = inputNodes.size() + outputNodes.size();
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
