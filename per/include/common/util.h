#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <common/graph.h>


using namespace std;
namespace fs = std::filesystem;


struct ReportData
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

    ReportData();
    ReportData(float _time, const string& dot_name, const string& dot_path,
               const string& placer, int cacheMisses, int tries, int swaps,
               const string& edges_algorithm, int total_cost,
               const vector<int>& placement, const vector<int>& n2c);

    string to_json() const;
};

template <typename T>
void randomVector(vector<T>& vec)
{
    random_device rd;
    mt19937 g(rd());
    shuffle(vec.begin(), vec.end(), g);
}

string funcKey(const string& a, const string& b);

string getProjectRoot();

string verifyPath(const string& path);

vector<pair<string, string>> getFilesListByExtension(
    const string& path,
    const string& file_extension
);

void saveToDot(const vector<pair<int, int>>& edges, const string& filename);

int calcGraphTotalDistance(const vector<int>& n2c, const vector<pair<int, int>>& edges, int nCellsSqrt);

int calcGraphLPDistance(const vector<int>& longestPath, const vector<int>& n2c, const int nCellsSqrt);

int getManhattanDist(int cell1, int cell2, int n_cells_sqrt);

void savePlacedDot(vector<int> n2c, vector<pair<int, int>> ed, int nCellsSqrt,
                   const string& filename);

void writeJson(const string& basePath,
               const string& reportPath,
               const string& algPath,
               const string& fileName,
               const ReportData& data);

void writeVprData(const string& basePath,
                  const string& reportPath,
                  const string& algPath,
                  const string& fileName,
                  const ReportData& data,
                  Graph g);

vector<vector<int>> getAdjCellsDist(int nCellsSqrt);

bool isInvalidCell(int cell, int nCellsSqrt);

bool isIOCell(int cell, int nCellsSqrt);

void createDir(const fs::path& pth);

int minBorderDist(int cell, int nCellsSqrt);
#endif
