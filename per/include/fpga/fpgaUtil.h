#ifndef FPGA_UTIL_H
#define FPGA_UTIL_H

#include <algorithm>
#include <fpga/fpgaGraph.h>
#include <common/util.h>

using namespace std;

struct FpgaReportData
{
    float _time;
    string dotName;
    string dotPath;
    string placer;
    int cacheMisses;
    int tries;
    int swaps;
    string edgesAlgorithm;
    int totalCost;
    vector<int> placement;
    vector<int> n2c;

    FpgaReportData();

    FpgaReportData(float _time, string dot_name, string dot_path,
                   string placer, int cacheMisses, int tries, int swaps,
                   string edges_algorithm, int total_cost,
                   const vector<int>& placement, const vector<int>& n2c);

    [[nodiscard]] string to_json() const;
};

void fpgaSavePlacedDot(vector<int>& n2c, const vector<pair<int, int>>& ed, int nCellsSqrt,
                       const string& filename);

vector<vector<int>> fpgaGetAdjCellsDist(int nCellsSqrt);

int fpgaCalcGraphTotalDistance(const vector<int>& n2c, const vector<pair<int, int>>& edges, int nCellsSqrt);

int fpgaCalcGraphLPDistance(const vector<int>& longestPath, const vector<int>& n2c, int nCellsSqrt);

bool fpgaIsInvalidCell(int cell, int nCellsSqrt);

bool fpgaIsIOCell(int cell, int nCellsSqrt);

int fpgaMinBorderDist(int cell, int nCellsSqrt);

void fpgaWriteJson(const string& basePath,
                   const string& reportPath,
                   const string& algPath,
                   const string& fileName,
                   const FpgaReportData& data);

void fpgaWriteVprData(const string& basePath,
                      const string& reportPath,
                      const string& algPath,
                      const string& fileName,
                      const FpgaReportData& data,
                      FPGAGraph g);
#endif
