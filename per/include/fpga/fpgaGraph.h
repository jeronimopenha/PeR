#ifndef FPGA_GRAPH_H
#define FPGA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>
#include <common/util.h>


class FPGAGraph : public Graph {
public:
    long nCells = 0;
    long nCellsSqrt = 0;

    //Neighbors vector
    std::vector<std::vector<long> > neighbors;

    std::vector<long> clbNodes;

    std::vector<long> longestPath;

    FPGAGraph(const std::string &dotPath, const std::string &dotName);

    void updateG();

    void readNeighbors();

    void calcMatrix();

    std::vector<long> getInOutPos() const;

    std::vector<long> getClbPos() const;

    std::unordered_map<std::string, std::vector<std::pair<long, long> > > fpgaGetGraphAnnotations(
        const std::vector<std::pair<long, long> > &edges,
        const std::vector<std::pair<long, long> > &convergences
    );

    void findLongestPath();

    std::vector<std::pair<long, long> > getEdgesDepthFirstPriority();
};

#endif
