#ifndef UTIL_H
#define UTIL_H

//Choose a type of total cost
#define TOTAL_COST
//#define LP_COST

//Choose the algorithm to be run
//#define YOTO_BASE_DF
//#define YOTO_BASE_DF_P
//#define YOTO_BASE_ZZ
//#define YOTO_BASE_ZZ_CACHE
#define YOTT_BASE

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include "graph.h"


using namespace std;

struct ReportData {
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

    ReportData()
        : _time(0), cacheMisses(0), tries(0), swaps(0), totalCost(0) {
    }

    // Constructor for easy initialization
    ReportData(float _time, const string &dot_name, const string &dot_path,
               const string &placer, int cacheMisses, int tries, int swaps, const string &edges_algorithm,
               int total_cost, const vector<int> &placement, const vector<int> &n2c)
        : _time(_time), dotName(dot_name), dotPath(dot_path), placer(placer), cacheMisses(cacheMisses), tries(tries),
          swaps(swaps), edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c) {
    }

    // Serialize ReportData to a JSON string
    string to_json() const {
        ostringstream oss;
        oss << "{\n"
                << "  \"time\": " << _time << ",\n"
                << "  \"dotName\": \"" << dotName << "\",\n"
                << "  \"dotPath\": \"" << dotPath << "\",\n"
                << "  \"placer\": \"" << placer << "\",\n"
                << "  \"cacheMisses\": " << cacheMisses << ",\n"
                << "  \"tries\": " << tries << ",\n"
                << "  \"swaps\": " << swaps << ",\n"
                << "  \"edgesAlgorithm\": \"" << edgesAlgorithm << "\",\n"
                << "  \"totalCost\": " << totalCost << ",\n"
                << "  \"placement\": [";

        // Serialize vector<int> placement
        for (size_t i = 0; i < placement.size(); ++i) {
            oss << placement[i];
            if (i < placement.size() - 1) oss << ", ";
        }
        oss << "],\n";

        // Serialize vector<int> n2c
        oss << "  \"n2c\": [";
        for (size_t i = 0; i < n2c.size(); ++i) {
            oss << n2c[i];
            if (i < n2c.size() - 1) oss << ", ";
        }
        oss << "]\n";

        oss << "}\n";
        return oss.str();
    }
};

string funcKey(const string &a, const string &b);

string getProjectRoot();

string verifyPath(const string &path);

vector<pair<string, string> > getFilesListByExtension(
    const string &path,
    const string &file_extension
);

void saveToDot(const vector<pair<int, int> > &edges, const string &filename);

template<typename T>
void randomVector(vector<T> &vec) {
    random_device rd;
    mt19937 g(rd());
    shuffle(vec.begin(), vec.end(), g);
}

int calcGraphTotalDistance(const vector<int> &n2c, const vector<pair<int, int> > &edges, int nCellsSqrt);

int calcGraphLPDistance(const vector<int> &longestPath, const vector<int> &n2c, const int nCellsSqrt);

int getManhattanDist(int cell1, int cell2, int n_cells_sqrt);

void savePlacedDot(vector<int> n2c, vector<pair<int, int> > ed, int nCellsSqrt,
                   const string &filename);

void writeJson(const string &basePath, const string &fileName, const ReportData &data);

void writeVprData(const string &basePath, const string &fileName, const ReportData &data, Graph g);

vector<vector<int> > getAdjCellsDist(int nCellsSqrt);


#endif //UTIL_H
