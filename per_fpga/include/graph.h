//
// Created by jeronimo on 27/12/24.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cmath>

class Graph {
public:
    std::string dotPath;
    std::string dotName;

    int nEdges = 0;
    int nNodes = 0;
    int nCells = 0;
    int nCellsSqrt = 0;

    //Adjacency for successors
    std::vector<std::vector<bool> > successors;
    //Adjacency for predecessors
    std::vector<std::vector<bool> > predecessors;
    //Edges list
    std::vector<std::pair<int, int> > gEdges;
    //input nodes
    std::vector<int> nSuccV;
    //output nodes
    std::vector<int> nPredV;
    std::vector<int> inputNodes;
    std::vector<int> outputNodes;


    Graph(const std::string &dotPath, const std::string &dotName);

    //void graphClearData();

    void getGraphDataStr();

    void getGraphDataInt();

    std::vector<int> getInOutPos();

    std::vector<std::pair<int, int> > getEdgesDepthFirst();

    std::vector<std::pair<int, int> > getEdgesDepthFirstPriority();

    std::vector<std::pair<int, int> > getEdgesZigzag();

    std::vector<std::pair<int, int> > clearEdges(const std::vector<std::pair<int, int> > &edges);
};


#endif //GRAPH_H
