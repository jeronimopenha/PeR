#ifndef QCA_UTIL_H
#define QCA_UTIL_H

#include <fstream>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;


struct QcaReportData {
    bool success;
    float _time;
    string dotName;
    string dotPath;
    string placer;
    int wires;
    int nodes;
    int tries;
    int swaps;
    int extraLayers;
    vector<int> extraLayersLevels;
    vector<int> placement;
    vector<int> n2c;

    QcaReportData();

    QcaReportData(bool success, float _time, string dotName, string dotPath, string placer,
                  int wires, int nodes, int tries, int swaps,
                  int extraLayers, vector<int> extraLayersLevels, vector<int> placement,
                  vector<int> n2c);

    string to_json() const;
};

vector<pair<int, int> > qcaGetInputDirections(int x, int y);

vector<pair<int, int> > qcaGetOutputDirections(int x, int y);

bool qcaIsInvalidCell(int x, int y, int nCellsSqrt);

void qcaExportUSEToDot(const string &filename, const vector<int> &n2c, const vector<pair<int, int> > &edges,
                       int nCellsSqrt);

#endif
