#ifndef FPGA_SA_H
#define FPGA_SA_H


#include <fpgaGraph.h>
#include <fpgaUtil.h>


ReportData fpgaSa(FPGAGraph &g);

void GetSwapCost(
    const std::vector<long> &n2c,
    long a,
    long b,
    long cellA,
    long cellB,
    long nCellsSqrt,
    const std::vector<std::vector<long> > &neighbors,
    long &costABefore,
    long &costAAfter,
    long &costBBefore,
    long &costBAfter
);

#endif
