//
// Created by jeronimo on 01/11/24.
//

#ifndef GRAPH_H
#define GRAPH_H
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include<chrono>
#include <algorithm>
#include <cmath>

class Graph {
public:
    std::string dotPath;
    std::string dotName;

    std::unordered_map<std::string, std::unordered_set<std::string> > succ;
    std::unordered_map<std::string, std::unordered_set<std::string> > succIdx;
    std::unordered_map<std::string, std::unordered_set<std::string> > pred;
    std::unordered_map<std::string, std::unordered_set<std::string> > predIdx;
    int n_edges = 0;
    std::vector<std::string> nodesStr;
    std::unordered_map<std::string, int> nodesToIdx;
    std::unordered_map<int, std::string> idxToNodes;
    std::vector<std::pair<std::string, std::string> > edgesStr;
    std::vector<std::pair<int, int> > edgesIdx;
    std::vector<std::string> inputNodesStr;
    std::vector<std::string> outputNodesStr;
    std::vector<int> inputNodesIdx;
    std::vector<int> outputNodesIdx;
    int nNodes = 0;
    int nCells = 0;
    int nCellsSqrt = 0;

    //std::unordered_map<int, std::vector<std::string> > neighbors_str;
    //std::unordered_map<int, std::vector<int> > neighbors_idx;
    //std::unordered_map<int, std::vector<int> > dir_neighbors_str;
    //std::unordered_map<int, std::vector<int> > dir_neighbors_idx;
    //std::vector<int> longest_path;
    //std::vector<int> longest_path_nodes;


    Graph(const std::string &dotPath, const std::string &dotName);

    void getGraphData();

    std::vector<std::pair<int, int> > getEdgesIdx(const std::vector<std::pair<std::string, std::string> > &edgesStr);

    std::vector<int> getNodesIdx(const std::vector<std::string> &nodesStr);

    std::vector<std::vector<std::vector<int> > > getMeshDistances();

    static std::string funcKey(const std::string &val1, const std::string &val2);

    static std::vector<std::string> funcUnkey(const std::string &text);

    std::vector<std::pair<std::string, std::string> > getEdgesDepthFirst();
};


#endif //GRAPH_H
