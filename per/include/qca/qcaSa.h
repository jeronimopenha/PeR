#ifndef QCA_SA_H
#define QCA_SA_H

#include <common/parameters.h>
#include <qca/qcaGraph.h>
#include <qca/qcaUtil.h>

using namespace std;

QcaReportData qcaSa(QCAGraph &g);

float qcaGetSwapCost(
    const std::vector<int> &n2c,
    int a,
    int b
);

#endif
