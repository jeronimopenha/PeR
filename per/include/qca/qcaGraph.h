#ifndef QCA_GRAPH_H
#define QCA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>

class QCAGraph : public Graph {
public:
    int nCells = 0;
    int nCellsSqrt = 0;

    int minOutputLevel = 0;
    int extraLayers = 0;
    vector<int> extraLayersLevels;

    unordered_map<int, int> level;
    unordered_map<int, string> dummyMap;
    unordered_map<int, vector<int> > levelSuccessors;
    unordered_map<int, vector<int> > levelPredecessors;

    vector<std::vector<int> > inNeighbors;
    vector<std::vector<int> > outNeighbors;

    QCAGraph(const string &dotPath, const string &dotName);

    void calcMatrix();

    void fixFanout();

    void fixFanin();

    void updateG();

    void updateNeighbors();

    void computeLevels();

    void balanceGraphAll();

    void exportUpGToDot(const string &filename);

    void saveDummyMap(const string &filename);

    bool verifyPlacement(const vector<int> &n2c, const vector<pair<int, int> > &edges,
                         int *invalidEdgesCount = nullptr) const;

    void insertDummyLayerAtLevel(int targetLevel);
};

#endif
