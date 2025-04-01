#include <qca/qcaUtil.h>


QcaReportData::QcaReportData()
    : _time(0), cacheMisses(0), tries(0), swaps(0), totalCost(0) {
}

// Constructor for easy initialization
QcaReportData::QcaReportData(float _time, const string &dot_name, const string &dot_path,
                               const string &placer, int cacheMisses, int tries, int swaps,
                               const string &edges_algorithm,
                               int total_cost, const vector<int> &placement, const vector<int> &n2c)
    : _time(_time), dotName(dot_name), dotPath(dot_path), placer(placer), cacheMisses(cacheMisses), tries(tries),
      swaps(swaps), edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c) {
}

// Serialize ReportData to a JSON string
string QcaReportData::to_json() const {
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

/*string getProjectRoot() {
    filesystem::path path = filesystem::current_path();
    for (int i = 0; i < 2; ++i) {
        path = path.parent_path();
    }
    return path.string();
}

string verifyPath(const string &path) {
    if (!path.empty() && path.back() != '/') {
        return path + '/';
    }
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

void saveToDot(const vector<pair<int, int> > &edges, const string &filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error!" << endl;
        return;
    }

    // write the dot header
    file << "digraph G {" << endl;

    // write the edges
    for (const auto &[fst, snd]: edges) {
        file << "    " << fst << " -> " << snd << ";" << endl;
    }

    // write the dot footer
    file << "}" << endl;

    file.close();
    cout << "File " << filename << " saved!" << endl;
}


int fpgaCalcGraphTotalDistance(const vector<int> &n2c, const vector<pair<int, int> > &edges,
                               const int nCellsSqrt) {
    int totalDist = -static_cast<int>(edges.size());

    for (const auto &[fst, snd]: edges) {
        const int tempDist = getManhattanDist(n2c[fst], n2c[snd], nCellsSqrt);

        // Acc the total distance
        totalDist += tempDist;
    }

    return totalDist;
}

int fpgaCalcGraphLPDistance(const vector<int> &longestPath, const vector<int> &n2c, const int nCellsSqrt) {
    int totalDist = 0;

    for (int idx = 0; idx < longestPath.size() - 1; idx++) {
        const int tempDist = getManhattanDist(n2c[longestPath[idx]], n2c[longestPath[idx + 1]], nCellsSqrt);

        // Acc the total distance
        totalDist += tempDist;
    }

    return totalDist;
}

int getManhattanDist(const int cell1, const int cell2, const int n_cells_sqrt) {
    const int cell1_x = cell1 % n_cells_sqrt;
    const int cell1_y = cell1 / n_cells_sqrt;
    const int cell2_x = cell2 % n_cells_sqrt;
    const int cell2_y = cell2 / n_cells_sqrt;
    return abs(cell1_y - cell2_y) + abs(cell1_x - cell2_x);
}


bool isInvalidCell(const int cell, const int nCellsSqrt) {
    const int l = cell / nCellsSqrt;
    const int c = cell % nCellsSqrt;

    bool outOfBounds = (l < 0 || l >= nCellsSqrt || c < 0 || c >= nCellsSqrt);

    const bool isCorner =
            (l == 0 && c == 0) ||
            (l == 0 && c == nCellsSqrt - 1) ||
            (l == nCellsSqrt - 1 && c == 0) ||
            (l == nCellsSqrt - 1 && c == nCellsSqrt - 1);

    return outOfBounds || isCorner;
}

bool isIOCell(const int cell, const int nCellsSqrt) {
    const int l = cell / nCellsSqrt;
    const int c = cell % nCellsSqrt;
    return l == 0 || l == nCellsSqrt - 1 || c == 0 || c == nCellsSqrt - 1;
}

void createDir(const fs::path &pth) {
    if (!exists(pth)) {
        if (create_directories(pth)) {
            cout << "Pastas criadas: " << pth << "\n";
        } else {
            cerr << "Erro ao criar o diretório: " << pth << "\n";
        }
    }
}*/
