#ifndef FPGA_SA_H
#define FPGA_SA_H

#include <common/parameters.h>
#include <fpga/fpgaGraph.h>
#include <fpga/fpgaUtil.h>

using namespace std;

FpgaReportData fpgaSa(FPGAGraph &g);

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
