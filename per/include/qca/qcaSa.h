#ifndef QCA_SA_H
#define QCA_SA_H


#include <qca/qcaGraph.h>
#include <qca/qcaUtil.h>

using namespace std;

struct QcaCost
{
    float directionPenalty = 0.0f;
    float distancePenalty = 0.0f;
    float inversionPenalty = 0.0f;
    float bonus = 0.0f; // para acertos perfeitos

    float total() const
    {
        return directionPenalty + distancePenalty + inversionPenalty - bonus;
    }
};


QcaReportData qcaSa(QCAGraph& g);

pair<float, float> qcaGetSwapCost(
    long cellNode1,
    long cellNode2,
    long node1,
    long node2,
    const vector<long>& n2c,
    const QCAGraph& g
);

tuple<float, float> qcaGetSwapCostImproved(
    long cellA, long cellB,
    long nodeA, long nodeB,
    const vector<long>& n2c,
    const QCAGraph& g
);

QcaCost computeQcaCostForNode(
    long node,
    long nodeCell,
    long otherNode,
    long otherCell,
    const vector<long>& n2c,
    const QCAGraph& g
);

#endif
