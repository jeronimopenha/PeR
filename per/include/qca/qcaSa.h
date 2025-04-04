#ifndef QCA_SA_H
#define QCA_SA_H

#include <common/parameters.h>
#include <qca/qcaGraph.h>
#include <qca/qcaUtil.h>

using namespace std;

QcaReportData qcaSa(QCAGraph &g);

pair<float, float> qcaGetSwapCost(
    int cellNode1,
    int cellNode2,
    int node1,
    int node2,
    const vector<int>& n2c,
    const QCAGraph &g
);

#endif
