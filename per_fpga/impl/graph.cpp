#include "graph.h"

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <vector>


void getGraphData() {
    succ = std::vector<std::vector<int>>(10, std::vector<int>(10));
    std::ifstream dotFile(dotPath);
    std::string line;

    std::unordered_map<std::string, std::unordered_set<std::string> > succ_str;
    std::unordered_map<std::string, std::unordered_set<std::string> > pred_str;
    std::unordered_set<std::string> nodesStr;
    std::unordered_map<std::string, int> nodesToIdx;
    std::unordered_map<int, std::string> idxToNodes;

    std::vector<std::pair<std::string, std::string> > edgesStr;
    std::vector<std::string> inputNodesStr;
    std::vector<std::string> outputNodesStr;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        std::cerr << "Error opening file: " << dotPath << std::endl;
        return;
    }

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
            size_t pos = toNode.find('l');
            if (pos != std::string::npos)
                toNode.erase(pos, 1);
            // Add the edge to the adjacency list

            nodesStr.insert(fromNode);
            nodesStr.insert(toNode);
            succ_str[fromNode].insert(toNode);
            edgesStr.emplace_back(fromNode, toNode);
        }
    }
    dotFile.close();

    //pred
    for (const auto &[fst, snd]: succ_str) {
        for (const auto &neighbor: snd) {
            pred_str[neighbor].insert(fst);
        }
    }


    //nodestoidx and idxtonodes
    //inputNodes str and idx
    //outputNodes str and idx
    succ = std::vector<std::vector<int>>(nodesStr.size(),std::vector<int>(nodesStr.size()));
    pred = std::vector<std::vector<int>>(nodesStr.size(),std::vector<int>(nodesStr.size()));
    int counter = 0;
    for (const auto &node: nodesStr) {
        nodesToIdx[node] = nNodes;
        idxToNodes[nNodes] = node;

        if (auto it = succ.find(node); it == succ.end()) {
            succ[node] = std::unordered_set<std::string>();
            outputNodesStr.push_back(node);
            outputNodesIdx.push_back(nodesToIdx[node]);
        }
        if (auto it = pred.find(node); it == pred.end()) {
            pred[node] = std::unordered_set<std::string>();
            inputNodesStr.push_back(node);
            inputNodesIdx.push_back(nodesToIdx[node]);
        }
        nNodes++;
    }

    //edgesIdx
    for (const auto &[fst, snd]: edgesStr) {
        edgesIdx.emplace_back(nodesToIdx[fst], nodesToIdx[snd]);
    }

    int totalInOut = inputNodesIdx.size() + outputNodesIdx.size();
    int nBaseNodes = nodesStr.size() - totalInOut;
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
