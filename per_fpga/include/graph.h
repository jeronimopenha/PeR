//
// Created by jeronimo on 27/12/24.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <vector>


inline std::string dotPath;
inline std::string dotName;

inline int n_edges = 0;
inline int nNodes = 0;
inline int nCells = 0;
inline int nCellsSqrt = 0;

inline std::vector<std::vector<int>> succ;
inline std::vector<std::vector<int>> pred;
inline std::vector<std::pair<int, int> > edgesIdx;
inline std::vector<int> inputNodesIdx;
inline std::vector<int> outputNodesIdx;

void getGraphData();

#endif //GRAPH_H
