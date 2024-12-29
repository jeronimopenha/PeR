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


inline std::string dotPath;
inline std::string dotName;

inline int nEdges = 0;
inline int nNodes = 0;
inline int nCells = 0;
inline int nCellsSqrt = 0;

//Adjacency for successors
inline std::vector<std::vector<bool>> successors;
//Adjacency for predecessors
inline std::vector<std::vector<bool>> predecessors;
//Edges list
inline std::vector<std::pair<int, int> > gEdges;
//input nodes
inline std::vector<int> nSuccV;
//output nodes
inline std::vector<int> nPredV;
inline std::vector<int> inputNodes;
inline std::vector<int> outputNodes;

void getGraphDataStr();

void getGraphDataInt();

std::vector<std::pair<int, int> > getEdgesDepthFirst();

std::vector<int> getInOutPos();

#endif //GRAPH_H
