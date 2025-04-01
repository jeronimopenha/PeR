#ifndef QCA_GRAPH_H
#define QCA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>

class QCAGraph : public Graph {
public:
    int minOutputLevel;
    unordered_map<int, int> level;
    unordered_map<int, string> dummyMap;
    unordered_map<int, vector<int> > levelSuccessors;
    unordered_map<int, vector<int> > levelPredecessors;

    QCAGraph(const string &dotPath, const string &dotName);

    void computeLevels();

    void balanceGraphAll();

    void exportUpGToDot(const string &filename);

    void saveDummyMap(const string &filename);

    void insertDummyLayerAtLevel(int targetLevel);
};

#endif
