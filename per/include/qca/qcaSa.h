#ifndef QCA_SA_H
#define QCA_SA_H

#include <common/parameters.h>
#include <qca/qcaGraph.h>
#include <qca/qcaUtil.h>

using namespace std;

struct QcaCost {
    float directionPenalty = 0.0f;
    float distancePenalty = 0.0f;
    float inversionPenalty = 0.0f;
    float bonus = 0.0f; // para acertos perfeitos

    float total() const {
        return directionPenalty + distancePenalty + inversionPenalty - bonus;
    }
};

QcaReportData qcaSa(QCAGraph &g);

pair<float, float> qcaGetSwapCost(
    int cellNode1,
    int cellNode2,
    int node1,
    int node2,
    const vector<int>& n2c,
    const QCAGraph &g
);

tuple<float, float> qcaGetSwapCostImproved(
    int cellA, int cellB,
    int nodeA, int nodeB,
    const vector<int>& n2c,
    const QCAGraph& g
);

QcaCost computeQcaCostForNode(
    int node,
    int nodeCell,
    int otherNode,
    int otherCell,
    const vector<int>& n2c,
    const QCAGraph& g
);

#endif
