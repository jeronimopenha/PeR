
#ifndef QCA_GRAPH_H
#define QCA_GRAPH_H

#include <unordered_map>
#include <common/graph.h>

class QCAGraph: public Graph{
public:

    QCAGraph(const string& dotPath, const string& dotName);

    unordered_map<int, int> computeLevels(const vector<int>& inputNodes, const vector<pair<int, int>>& edges);

    void balanceGraph();
};

#endif //QCAGRAPH_H
