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
        : execId(0), tries(0), swaps(0), totalCost(0) {
    }

    // Constructor for easy initialization
    ReportData(int execId, const std::string &dot_name, const std::string &dot_path, const std::string &placer,
               int tries, int swaps, const std::string &edges_algorithm, int total_cost,
               const std::vector<int> &placement, const std::vector<int> &n2c)
        : execId(execId), dotName(dot_name), dotPath(dot_path), placer(placer), tries(tries), swaps(swaps),
          edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c) {
    }
};

std::string getProjectRoot();

std::string verifyPath(const std::string &path);

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension);

int get_manhattan_distance(int cell1, int cell2, int n_cells_sqrt);

void generate_vpr_data(Graph &graph, const ReportData &data, const std::string &netPath, const std::string &placePath);
#endif //UTIL_H
