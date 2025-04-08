#ifndef FPGA_GRAPH_H
#define FPGA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>
#include <common/util.h>


class FPGAGraph : public Graph
{
public:
    int nCells = 0;
    int nCellsSqrt = 0;

    //Neighbors vector
    vector<vector<int>> neighbors;

    vector<int> clbNodes;

    vector<int> longestPath;

    FPGAGraph(const string& dotPath, const string& dotName);

    void updateG();

    void readNeighbors();

    void calcMatrix();

    vector<int> getInOutPos() const;

    vector<int> getClbPos() const;

    unordered_map<string, vector<pair<int, int>>> fpgaGetGraphAnnotations(
        const vector<pair<int, int>>& edges,
        const vector<pair<int, int>>& convergences
    );

    void findLongestPath();

    vector<pair<int, int>> getEdgesDepthFirstPriority();
};

#endif
