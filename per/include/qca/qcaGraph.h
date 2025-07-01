#ifndef QCA_GRAPH_H
#define QCA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>

class QCAGraph : public Graph {
public:
    long nCells = 0;
    long nCellsSqrt = 0;

    long minOutputLevel = 0;
    long numLevels = 0;
    long extraLayers = 0;
    std::vector<long> extraLayersLevels;

    std::unordered_map<long, long> level;
    std::unordered_map<long, std::string> dummyMap;
    std::unordered_map<long, std::vector<long> > levelSuccessors;
    std::unordered_map<long, std::vector<long> > levelPredecessors;

    std::vector<std::vector<long> > inNeighbors;
    std::vector<std::vector<long> > outNeighbors;

    QCAGraph(const std::string &dotPath, const std::string &dotName);

    void calcMatrix();

    void fixFanout();

    void fixFanin();

    void updateG();

    void updateNeighbors();

    void computeLevels();

    void balanceGraphAll();

    void exportUpGToDot(const std::string &filename);

    void saveDummyMap(const std::string &filename);

    bool verifyPlacement(const std::vector<long> &n2c, const std::vector<std::pair<long, long> > &edges,
                         long *invalidEdgesCount = nullptr) const;

    std::unordered_map<std::string, std::vector<std::pair<long, long> > > qcaGetGraphAnnotations(const std::vector<std::pair<long, long> > &edges,
                                                                           const std::vector<std::pair<long, long> > &convergences);

    std::vector<long> getInterleavedOutputCellsQCA() const;

    void insertDummyLayerAtLevel(long targetLevel);
};

#endif
