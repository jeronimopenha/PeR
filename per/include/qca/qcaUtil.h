#ifndef QCA_UTIL_H
#define QCA_UTIL_H

#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <random>

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


#endif
