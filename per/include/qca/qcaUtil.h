#ifndef QCA_UTIL_H
#define QCA_UTIL_H

#include <fstream>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;


struct QcaReportData
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

    QcaReportData();
    QcaReportData(float _time, const string& dot_name, const string& dot_path,
                  const string& placer, int cacheMisses, int tries, int swaps,
                  const string& edges_algorithm, int total_cost,
                  const vector<int>& placement, const vector<int>& n2c);

    string to_json() const;
};

vector<pair<int, int>> qcaGetInputDirections(int x, int y);

vector<pair<int, int>> qcaGetOutputDirections(int x, int y);

bool qcaIsInvalidCell(int x, int y, int nCellsSqrt);

void qcaExportUSEToDot(const string& filename, const vector<int>& n2c, int nCellsSqrt);

#endif
