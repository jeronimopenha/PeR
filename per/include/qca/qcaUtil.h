#ifndef QCA_UTIL_H
#define QCA_UTIL_H

#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


struct QcaReportData {
    bool success = false;
    bool allPLaced = false;
    double _time = 0.0f;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    long nCellsSqrt = 0;
    long wires = 0;
    long nNodes = 0;
    long tries = 0;
    long swaps = 0;
    long wrongEdges = 0;
    long area = 0;
    double usedAreaPercentage = 0.0f;
    long extraLayers = 0;
    std::vector<long> extraLayersLevels;
    std::vector<long> placement;
    std::vector<long> n2c;
    std::vector<std::pair<long, long> > edges;

    QcaReportData();

    QcaReportData(bool success, bool allPLaced, double _time, std::string dotName, std::string dotPath,
                  std::string placer, long nCellsSqrt, long wires, long nNodes, long tries, long swaps, long wrongEdges,
                  long area, double usedAreaPercentage, long extraLayers, std::vector<long> extraLayersLevels,
                  std::vector<long> placement, std::vector<long> n2c, std::vector<std::pair<long, long> > edges);

    [[nodiscard]] std::string to_json() const;
};

struct AreaMetrics {
    long minRow = 0;
    long maxRow = 0;
    long minCol = 0;
    long maxCol = 0;
    long occupiedCells = 0;
    long totalCells = 0;
    double utilization = 0.0f;
};

std::vector<std::pair<long, long> > qcaGetInputDirections(long x = 0, long y = 0);

std::vector<std::pair<long, long> > qcaGetOutputDirections(long x = 0, long y = 0);

bool qcaIsInvalidCell(long x, long y, long nCellsSqrt);

void qcaExportUSEToDot(const std::string &filename, const std::vector<long> &n2c,
                       const std::vector<std::pair<long, long> > &edges,
                       long nCellsSqrt);

AreaMetrics computeOccupiedAreaMetrics(long nCellsSqrt, const std::vector<long> &c2n);

void qcaWriteJson(const std::string &basePath,
                  const std::string &reportPath,
                  const std::string &algPath,
                  const std::string &fileName,
                  long extraLayers,
                  const QcaReportData &data);

bool allPlaced(const std::vector<long> &n2c);

#endif
