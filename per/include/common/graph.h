#ifndef GRAPH_H
#define GRAPH_H
#include <string>


using namespace std;

class Graph
{
public:
    string dotPath;
    string dotName;

    int nEdges = 0;
    int nNodes = 0;
    int nCells = 0;
    int nCellsSqrt = 0;

    Graph(const string& dotPath, const string& dotName);

};
#endif //GRAPH_H
