#include   "util.h"

std::string getProjectRoot() {
    std::filesystem::path path = std::filesystem::current_path();
    for (int i = 0; i < 2; ++i) {
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

void saveToDot(const std::vector<std::pair<int, int> > &edges, const std::string &filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error!" << std::endl;
        return;
    }

    // write the dot header
    file << "digraph G {" << std::endl;

    // write the edges
    for (const auto &[fst, snd]: edges) {
        file << "    " << fst << " -> " << snd << ";" << std::endl;
    }

    // write the dot footer
    file << "}" << std::endl;

    file.close();
    std::cout << "File " << filename << " saved!" << std::endl;
}
