#ifndef FPGA_SA_H
#define FPGA_SA_H



#include <fpga/fpgaGraph.h>
#include <fpga/fpgaUtil.h>


FpgaReportData fpgaSa(FPGAGraph &g);

void fpgaGetSwapCost(
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
