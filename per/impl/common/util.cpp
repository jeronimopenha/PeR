#include <common/util.h>

using namespace std;

string getProjectRoot() {
    filesystem::path path = filesystem::current_path();
    for (int i = 0; i < 2; ++i)
        path = path.parent_path();

    return path.string();
}

string verifyPath(const string &path) {
    if (!path.empty() && path.back() != '/')
        return path + '/';

    return path;
}

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

string funcKey(const string &a, const string &b) {
    return a + " " + b;
}

void saveToDot(const vector<pair<long, long> > &edges, const string &filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error!" << endl;
        return;
    }

    // write the dot header
    file << "digraph G {" << endl;

    // write the edges
    for (const auto &[fst, snd]: edges)
        file << "    " << fst << " -> " << snd << ";" << endl;

    // write the dot footer
    file << "}" << endl;

    file.close();
    cout << "File " << filename << " saved!" << endl;
}


long getManhattanDist(const long cell1, const long cell2, const long n_cells_sqrt) {
    const long cell1_x = cell1 % n_cells_sqrt;
    const long cell1_y = cell1 / n_cells_sqrt;
    const long cell2_x = cell2 % n_cells_sqrt;
    const long cell2_y = cell2 / n_cells_sqrt;
    return abs(cell1_y - cell2_y) + abs(cell1_x - cell2_x);
}


void createDir(const fs::path &pth) {
    if (!exists(pth)) {
        if (create_directories(pth))
            cout << "Pastas criadas: " << pth << "\n";
        else
            cerr << "Erro ao criar o diretório: " << pth << "\n";
    }
}

long getX(const long cellIndex, const long nCellsSqrt) {
    return cellIndex % nCellsSqrt;
}

long getY(const long cellIndex, const long nCellsSqrt) {
    return cellIndex / nCellsSqrt;
}

long getCellIndex(const long x, const long y, const long nCellsSqrt) {
    return y * nCellsSqrt + x;
}

int randomInt(const int min, const int max) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

float randomFloat(const float min, const float max) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> distrib(min, max);
    return distrib(gen);
}
