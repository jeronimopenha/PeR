#ifndef QCA_UTIL_H
#define QCA_UTIL_H

#include <fstream>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;


struct QcaReportData
{
    bool success;
    bool allPLaced;
    float _time;
    string dotName;
    string dotPath;
    string placer;
    int nCellsSqrt;
    int wires;
    int nNodes;
    int tries;
    int swaps;
    int wrongEdges;
    int area;
    float usedAreaPercentage;
    int extraLayers;
    vector<int> extraLayersLevels;
    vector<int> placement;
    vector<int> n2c;
    vector<pair<int, int>> edges;

    QcaReportData();

    QcaReportData(bool success, bool allPLaced, float _time, string dotName, string dotPath, string placer,
                  int nCellsSqrt, int wires, int nNodes, int tries, int swaps,
                  int wrongEdges, int area, float usedAreaPercentage, int extraLayers,
                  vector<int> extraLayersLevels, vector<int> placement,
                  vector<int> n2c, vector<pair<int, int>> edges);

    [[nodiscard]] string to_json() const;
};

struct AreaMetrics
{
    int minRow, maxRow;
    int minCol, maxCol;
    int occupiedCells;
    int totalCells;
    float utilization;
};

vector<pair<int, int>> qcaGetInputDirections(int x, int y);

vector<pair<int, int>> qcaGetOutputDirections(int x, int y);

bool qcaIsInvalidCell(int x, int y, int nCellsSqrt);

void qcaExportUSEToDot(const string& filename, const vector<int>& n2c, const vector<pair<int, int>>& edges,
                       int nCellsSqrt);

AreaMetrics computeOccupiedAreaMetrics(int nCellsSqrt, const vector<int>& c2n);

void qcaWriteJson(const string& basePath,
                  const string& reportPath,
                  const string& algPath,
                  const string& fileName,
                  int extraLayers,
                  const QcaReportData& data);

bool allPLaced(const vector<int>& n2c);

#endif
