#ifndef FPGA_UTIL_H
#define FPGA_UTIL_H

#include <algorithm>
#include <fpga/fpgaGraph.h>
#include <common/util.h>


struct FpgaReportData {
    double _time = 0.0;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    long cacheMisses = 0;
    long tries = 0;
    long swaps = 0;
    std::string edgesAlgorithm;
    long totalCost = 0;
    std::vector<long> placement;
    std::vector<long> n2c;

    FpgaReportData();

    FpgaReportData(double _time, std::string dotName, std::string dotPath,
                   std::string placer, long cacheMisses, long tries,
                   long swaps, std::string edges_algorithm, long total_cost,
                   const std::vector<long> &placement, const std::vector<long> &n2c);

    [[nodiscard]] std::string to_json() const;
};

void fpgaSavePlacedDot(std::vector<long> &n2c, const std::vector<std::pair<long, long> > &ed, long nCellsSqrt,
                       const std::string &filename);

std::vector<std::vector<long> > fpgaGetAdjCellsDist(long nCellsSqrt);

long fpgaCalcGraphTotalDistance(const std::vector<long> &n2c, const std::vector<std::pair<long, long> > &edges,
                                long nCellsSqrt);

long fpgaCalcGraphLPDistance(const std::vector<long> &longestPath, const std::vector<long> &n2c,
                             long nCellsSqrt);

bool fpgaIsInvalidCell(long cell, long nCellsSqrt);

bool fpgaIsIOCell(long cell, long nCellsSqrt);

long fpgaMinBorderDist(long cell, long nCellsSqrt);

void fpgaWriteJson(const std::string &basePath,
                   const std::string &reportPath,
                   const std::string &algPath,
                   const std::string &fileName,
                   const FpgaReportData &data);

void fpgaWriteVprData(const std::string &basePath,
                      const std::string &reportPath,
                      const std::string &algPath,
                      const std::string &fileName,
                      const FpgaReportData &data,
                      FPGAGraph g);
#endif
