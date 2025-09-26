#include <common/util.h>

namespace fs = std::filesystem;
using namespace std;

/**
 * Return the full path from root to executable folder
 * @return
 */
string getProjectRoot() {
    filesystem::path path = filesystem::current_path();
    for (int i = 0; i < 2; ++i)
        path = path.parent_path();

    return path.string();
}

/**
 * Verifies if the path contains the "/" character in the end
 * @param path
 * @return
 */
string verifyPath(const string &path) {
    if (!path.empty() && path.back() != '/')
        return path + '/';

    return path;
}

/**
 *  Returns the list of files inside a folder that has the chose extension
 * @param path
 * @param file_extension
 * @return
 */
vector<pair<string, string> > getFilesListByExtension(
    const string &path, const string &file_extension) {
    vector<pair<string, string> > files_list_by_extension;

    for (const auto &entry: filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == file_extension) {
            string file_path = entry.path().string();
            string file_name = entry.path().filename().string();
            files_list_by_extension.emplace_back(file_path, file_name);
        }
    }

    return files_list_by_extension;
}

/**
 * Just put a " " between the variables a and b and return the string.
 * Used to make keys for a map
 * @param a
 * @param b
 * @return
 */
string funcKey(const string &a, const string &b) {
    return a + " " + b;
}


/**
 * Calculates the distance between two cells by the vector position
 * @param cell1
 * @param cell2
 * @param n_cells_sqrt
 * @return
 */
long getManhattanDist(const long cell1, const long cell2, const long n_cells_sqrt) {
    const long cell1_x = cell1 % n_cells_sqrt;
    const long cell1_y = cell1 / n_cells_sqrt;
    const long cell2_x = cell2 % n_cells_sqrt;
    const long cell2_y = cell2 / n_cells_sqrt;
    return abs(cell1_y - cell2_y) + abs(cell1_x - cell2_x);
}


/**
 * A simple function to crate the needed directories always they are needed
 * @param path
 */
void createDir(const fs::path &path) {
    if (!exists(path)) {
        if (create_directories(path))
            cout << "The folders were created: " << path << "\n";
        else
            cerr << "Error while creating folders: " << path << "\n";
    }
}


/**
 * Return the column from a vector index
 * @param cellIndex
 * @param nCellsSqrt
 * @return
 */
long getColumn(const long cellIndex, const long nCellsSqrt) {
    return cellIndex % nCellsSqrt;
}

/**
 * Return the line from a vector index
 * @param cellIndex
 * @param nCellsSqrt
 * @return
 */
long getLine(const long cellIndex, const long nCellsSqrt) {
    return cellIndex / nCellsSqrt;
}


/**
 * Return the vector index form column and line parameters
 * @param column
 * @param line
 * @param nCellsSqrt
 * @return
 */
long getCellIndex(const long column, const long line, const long nCellsSqrt) {
    return line * nCellsSqrt + column;
}


/**
 * Returns a random integer
 * @param min
 * @param max
 * @return
 */
int randomInt(const int min, const int max) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}


/**
 * Returns a custom float
 * @param min
 * @param max
 * @return
 */
float randomFloat(const float min, const float max) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> distrib(min, max);
    return distrib(gen);
}
