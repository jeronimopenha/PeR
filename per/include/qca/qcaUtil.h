#ifndef QCA_UTIL_H
#define QCA_UTIL_H

#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


struct QcaReportData {
    bool success = false;
    bool allPLaced = false;
    float _time = 0.0f;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    int nCellsSqrt = 0;
    int wires = 0;
    int nNodes = 0;
    int tries = 0;
    int swaps = 0;
    int wrongEdges = 0;
    int area = 0;
    float usedAreaPercentage = 0.0f;
    int extraLayers = 0;
    std::vector<int> extraLayersLevels;
    std::vector<int> placement;
    std::vector<int> n2c;
    std::vector<std::pair<int, int> > edges;

    //QcaReportData();

    QcaReportData(bool success, bool allPLaced, float _time, std::string dotName, std::string dotPath, std::string placer,
                  int nCellsSqrt, int wires, int nNodes, int tries, int swaps,
                  int wrongEdges, int area, float usedAreaPercentage, int extraLayers,
                  std::vector<int> extraLayersLevels, std::vector<int> placement,
                  std::vector<int> n2c, std::vector<std::pair<int, int> > edges);

    [[nodiscard]] std::string to_json() const;
};

struct AreaMetrics {
    int minRow = 0;
    int maxRow = 0;
    int minCol = 0;
    int maxCol = 0;
    int occupiedCells = 0;
    int totalCells = 0;
    float utilization = 0.0f;
};

std::vector<std::pair<int, int> > qcaGetInputDirections(int x = 0, int y = 0);

std::vector<std::pair<int, int> > qcaGetOutputDirections(int x = 0, int y = 0);

bool qcaIsInvalidCell(int x, int y, int nCellsSqrt);

void qcaExportUSEToDot(const std::string &filename, const std::vector<int> &n2c, const std::vector<std::pair<int, int> > &edges,
                       int nCellsSqrt);

AreaMetrics computeOccupiedAreaMetrics(int nCellsSqrt, const std::vector<int> &c2n);

void qcaWriteJson(const std::string &basePath,
                  const std::string &reportPath,
                  const std::string &algPath,
                  const std::string &fileName,
                  int extraLayers,
                  const QcaReportData &data);

bool allPlaced(const std::vector<int> &n2c);

#endif
