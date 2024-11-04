//
// Created by jeronimo on 01/11/24.
//

#ifndef UTIL_H
#define UTIL_H

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
    double totalCost;
    std::vector<int> histogram;
    std::vector<int> placement;
    std::vector<int> n2c;

    // Constructor for easy initialization
    ReportData(int execId, const std::string &dot_name, const std::string &dot_path, const std::string &placer,
               int tries, int swaps, const std::string &edges_algorithm, double total_cost,
               const std::vector<int> &histogram, const std::vector<int> &placement, const std::vector<int> &n2c)
        : execId(execId), dotName(dot_name), dotPath(dot_path), placer(placer), tries(tries), swaps(swaps),
          edgesAlgorithm(edges_algorithm), totalCost(total_cost), histogram(histogram), placement(placement), n2c(n2c) {
    }
};

std::string getProjectRoot();

std::string verifyPath(const std::string &path);

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension);

#endif //UTIL_H
