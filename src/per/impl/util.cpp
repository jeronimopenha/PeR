//
// Created by jeronimo on 01/11/24.
//
#include "util.h"

std::string getProjectRoot() {
    std::filesystem::path path = std::filesystem::current_path();
    for (int i = 0; i < 3; ++i) {
        path = path.parent_path();
    }
    return path.string();
}

std::string verifyPath(const std::string &path) {
    if (!path.empty() && path.back() != '/') {
        return path + '/';
    }
    return path;
}

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension) {
    std::vector<std::pair<std::string, std::string> > files_list_by_extension;

    for (const auto &entry: std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == file_extension) {
            std::string file_path = entry.path().string();
            std::string file_name = entry.path().filename().string();
            files_list_by_extension.emplace_back(file_path, file_name);
        }
    }

    return files_list_by_extension;
}

int get_manhattan_distance(const int cell1, const int cell2, const int n_cells_sqrt) {
    const int cell1_x = cell1 % n_cells_sqrt;
    const int cell1_y = cell1 / n_cells_sqrt;
    const int cell2_x = cell2 % n_cells_sqrt;
    const int cell2_y = cell2 / n_cells_sqrt;
    return std::abs(cell1_y - cell2_y) + std::abs(cell1_x - cell2_x);
}

static void write_json(const std::string &path, const std::string &fileName,
                       const ReportData &data) {
    const std::string verifiedPath = verifyPath(path);
    std::ofstream file(verifiedPath + fileName + ".json");

    if (file.is_open()) {
        file << data.to_json();; // Write JSON string to file
        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << fileName << ".json" << std::endl;
    }
}


void save_reports(const std::string &dotName, const std::string &path, const std::string &fileNamePref,
                  const std::unordered_map<int, ReportData> &reports) {
    for (const auto &[fst, snd]: reports) {
        const int exec_id = snd.execId;
        std::string fileName = dotName + "_" + fileNamePref + "_" + std::to_string(exec_id);
        write_json(path, fileName, snd);
    }
}
