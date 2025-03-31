#ifndef SA_H
#define SA_H

#include "parameters.h"
#include "util.h"
#include "graph.h"

using namespace std;

ReportData sa(Graph &g);

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
