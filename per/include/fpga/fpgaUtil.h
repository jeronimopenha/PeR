#ifndef FPGA_UTIL_H
#define FPGA_UTIL_H

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <algorithm>
#include <fpga/fpgaGraph.h>
#include <common/util.h>
#include <map>

struct RGB {
    int r, g, b;
};

struct FpgaReportData {
    double _time = 0.0;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    long cacheMisses = 0;
    long w = 0;
    long wCost = 0;
    long cachePenalties = 0;
    long clbTries = 0;
    long ioTries = 0;
    long tries = 0;
    long triesP = 0;
    long swaps = 0;
    std::string edgesAlgorithm;
    long totalCost = 0;
    long lPCost = 0;
    std::vector<long> placement;
    std::vector<long> n2c;
    std::vector<std::map<long, long> > hist;
    std::vector<long> heatEnd;
    std::vector<long> heatBegin;

    FpgaReportData();

    FpgaReportData(double _time, std::string dotName, std::string dotPath, std::string placer, long cacheMisses, long w,
                   long wCost, long cachePenalties, long clbTries, long ioTries, long tries, long triesP, long swaps,
                   std::string edges_algorithm, long totalCost, long lPCost, const std::vector<long> &placement,
                   const std::vector<long> &n2c, std::vector<std::map<long, long> > hist, std::vector<long> heatEnd,
                   std::vector<long> heatBegin);

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

RGB valueToRGB(const float normValue);

void generateHeatmap(const std::vector<long> &heatEnd,
                     const std::vector<long> &heatbegin,
                     long nCellsSqrt,
                     const std::string &basePath,
                     const std::string &reportPath,
                     const std::string &algPath,
                     const std::string &fileName);

#endif
