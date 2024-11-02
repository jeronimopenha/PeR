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

std::string verifyPath(const std::string& path) {
    if (!path.empty() && path.back() != '/') {
        return path + '/';
    }
    return path;
}

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension) {
    std::vector<std::pair<std::string, std::string>> files_list_by_extension;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == file_extension) {
            std::string file_path = entry.path().string();
            std::string file_name = entry.path().filename().string();
            files_list_by_extension.emplace_back(file_path, file_name);
        }
    }

    return files_list_by_extension;
}