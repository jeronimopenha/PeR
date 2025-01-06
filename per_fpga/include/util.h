#ifndef UTIL_H
#define UTIL_H

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


struct ReportData {
    float _time;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    int cacheMisses;
    int tries;
    int swaps;
    std::string edgesAlgorithm;
    int totalCost;
    std::vector<int> placement;
    std::vector<int> n2c;

    ReportData()
        : _time(0), cacheMisses(0), tries(0), swaps(0), totalCost(0) {
    }

    // Constructor for easy initialization
    ReportData(float _time, const std::string &dot_name, const std::string &dot_path,
               const std::string &placer, int cacheMisses, int tries, int swaps, const std::string &edges_algorithm,
               int total_cost, const std::vector<int> &placement, const std::vector<int> &n2c)
        : _time(_time), dotName(dot_name), dotPath(dot_path), placer(placer), cacheMisses(cacheMisses), tries(tries),
          swaps(swaps), edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c) {
    }

    // Serialize ReportData to a JSON string
    std::string to_json() const {
        std::ostringstream oss;
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

std::string getProjectRoot();

std::string verifyPath(const std::string &path);

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension);

void saveToDot(const std::vector<std::pair<int, int> > &edges, const std::string &filename);

template<typename T>
void randomVector(std::vector<T> &vec) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vec.begin(), vec.end(), g);
}

int calcGraphTotalDistance(const std::vector<int> &n2c, const std::vector<std::pair<int, int> > &edges, int nCellsSqrt);

int getManhattanDist(const int cell1, const int cell2, const int n_cells_sqrt);

void savePlacedDot(std::vector<int> n2c, std::vector<std::pair<int, int> > ed, int nCellsSqrt,
                   const std::string &filename);

void writeJson(const std::string &basePath, const std::string &fileName, const ReportData &data);

void writeVprData(const std::string &basePath, const std::string &fileName, const ReportData &data);
#endif //UTIL_H
