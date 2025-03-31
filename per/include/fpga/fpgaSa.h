#ifndef SA_H
#define SA_H

#include <common/parameters.h>
#include <common/util.h>
#include <common/graph.h>

using namespace std;

ReportData fpgaSa(Graph &g);

void getSwapCost(
    const std::vector<int> &n2c,
    int a,
    int b,
    int cellA,
    int cellB,
    int nCellsSqrt,
    const std::vector<std::vector<int> > &neighbors,
    int &costABefore,
    int &costAAfter,
    int &costBBefore,
    int &costBAfter
);

#endif
