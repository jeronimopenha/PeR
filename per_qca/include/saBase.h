//
// Created by jeronimo on 24/03/25.
//

#ifndef SA_BASE_H
#define SA_BASE_H

#include "util.h"
#include "graph.h"

using namespace std;

ReportData saBase(Graph &g);

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

#endif //SA_BASE_H
