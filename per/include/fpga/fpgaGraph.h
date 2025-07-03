#ifndef FPGA_GRAPH_H
#define FPGA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>
#include <common/util.h>

struct BorderInfo {
    long distance;
    //0 - top
    //1 - bottom
    //2 - left
    //3-  right
    int direction;
    std::pair<long, long> coord; // coordenada da borda mais próxima
};

class FPGAGraph : public Graph {
public:
    long nCells = 0;
    long nCellsSqrt = 0;

    //Neighbors vector
    std::vector<std::vector<long> > neighbors;

    FPGAGraph(const std::string &dotPath, const std::string &dotName);

    void readNeighbors();

    void calcMatrix();

    std::vector<long> getInOutPos() const;

    std::vector<long> getClbPos() const;

    std::unordered_map<std::string, std::vector<std::pair<long, long> > > fpgaGetGraphAnnotations(
        const std::vector<std::pair<long, long> > &edges,
        const std::vector<std::pair<long, long> > &convergences
    );

    std::vector<std::vector<long> > generateOffsets();

    std::vector<BorderInfo> getBordersSequence(long l, long c);

    std::vector<std::pair<long, long> > getEdgesDepthFirstPriority();
};

#endif
