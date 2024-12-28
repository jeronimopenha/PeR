//
// Created by jeronimo on 01/11/24.
//

#ifndef UTIL_H
#define UTIL_H

#include "graph.h"
#include <filesystem>
#include <string>
#include <vector>
#include <utility>

struct ReportData {
    int execId;
    float _time;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    int tries;
    int swaps;
    std::string edgesAlgorithm;
    int totalCost;
    std::vector<int> placement;
    std::vector<int> n2c;

    ReportData()
        : execId(0), _time(0), tries(0), swaps(0), totalCost(0) {
    }

    // Constructor for easy initialization
    ReportData(int execId, float _time, const std::string &dot_name, const std::string &dot_path,
               const std::string &placer,
               int tries, int swaps, const std::string &edges_algorithm, int total_cost,
               const std::vector<int> &placement, const std::vector<int> &n2c)
        : execId(execId), _time(_time), dotName(dot_name), dotPath(dot_path), placer(placer), tries(tries),
          swaps(swaps),
          edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c) {
    }

    // Serialize ReportData to a JSON string
    std::string to_json() const {
        std::ostringstream oss;
        oss << "{\n"
                << "  \"execId\": " << execId << ",\n"
                << "  \"time\": " << _time << ",\n"
                << "  \"dotName\": \"" << dotName << "\",\n"
                << "  \"dotPath\": \"" << dotPath << "\",\n"
                << "  \"placer\": \"" << placer << "\",\n"
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

int get_manhattan_distance(int cell1, int cell2, int n_cells_sqrt);

void save_reports(const std::string &dotName, const std::string &path, const std::string &fileNamePref,
                  const std::unordered_map<int, ReportData> &reports);

static void write_json(const std::string &path, const std::string &fileName,
                       const ReportData &data);

static void write_json(const std::string &path, const std::string &fileName,
                       const std::unordered_map<std::string, std::string> &data);
#endif //UTIL_H
