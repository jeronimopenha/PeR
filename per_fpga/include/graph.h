//
// Created by jeronimo on 27/12/24.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <utility>
#include <random>
#include<chrono>
#include <algorithm>
#include <cmath>


inline std::string dotPath;
inline std::string dotName;

inline int n_edges = 0;
inline int nNodes = 0;
inline int nCells = 0;
inline int nCellsSqrt = 0;

//The adjacency tables are matrices power by 2
//Adjacency for successors
inline std::vector<std::vector<int> > successors;
//Adjacency for predecessors
inline std::vector<std::vector<int> > predecessors;
//Edges list
inline std::vector<std::pair<int, int> > edges;
//input nodes
inline std::vector<int> inputNodes;
//output nodes
inline std::vector<int> outputNodes;

void getGraphData();

#endif //GRAPH_H
