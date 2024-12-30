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

void randomVector(std::vector<int> &vec) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(vec.begin(), vec.end(), g);
}

int calcGraphTotalDistance(const std::vector<int> &n2c, const std::vector<std::pair<int, int> > &edges,
                           const int nCellsSqrt) {
    int totalDist = -static_cast<int>(edges.size());

    for (const auto &[fst, snd]: edges) {
        const int tempDist = getManhattanDist(n2c[fst], n2c[snd], nCellsSqrt);

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
    return std::abs(cell1_y - cell2_y) + std::abs(cell1_x - cell2_x);
}

void savePlacedDot(std::vector<int> n2c, std::vector<std::pair<int, int> > ed, int nCellsSqrt,
                   const std::string &filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error!" << std::endl;
        return;
    }

    std::vector<int> cells(nCellsSqrt * nCellsSqrt, -1);

    for (int i = 0; i < n2c.size(); i++) {
        cells[n2c[i]] = i;
    }

    // write the dot header
    file << "digraph layout{" << std::endl;
    file << "rankdir=TB; \n" << std::endl;
    file << "splines=ortho; \n" << std::endl;
    file << "node [style=filled shape=square fixedsize=true width=0.6];" << std::endl;

    for (int i = 0; i < cells.size(); i++) {
        if (cells[i] == -1) {
            file << i << "[label=\"\", fontsize=8, fillcolor=\"#ffffff\"];" << std::endl;
        } else {
            file << i << "[label=\"" << cells[i] << "\", fontsize=8, fillcolor=\"#a9ccde\"];" << std::endl;
        }
    }
    file << "edge [constraint=false, style=vis];" << std::endl;
    //normal edges
    for (auto [fst,snd]: ed) {
        file << n2c[fst] << " -> " << n2c[snd] << ";" << std::endl;
    }


    file << "edge [constraint=true, style=invis];" << std::endl;
    //structural edges
    for (int j = 0; j < nCellsSqrt; j++) {
        for (int i = 0; i < nCellsSqrt; i++) {
            int c = j + i * nCellsSqrt;
            if (i == nCellsSqrt - 1) {
                file << c << ";" << std::endl;
            } else {
                file << c << " -> ";
            }
        }
    }

    for (int i = 0; i < nCellsSqrt; i++) {
        file << "rank = same { ";
        for (int j = 0; j < nCellsSqrt; j++) {
            int c = i * nCellsSqrt + j;
            if (j == nCellsSqrt - 1) {
                file << c << ";";
            } else {
                file << c << " -> ";
            }
        }
        file << "};" << std::endl;
    }


    // write the dot footer
    file << "}" << std::endl;
    file.close();
}

void writeJson(const std::string &basePath, const std::string &fileName, const ReportData &data) {
    std::string jsonFile = basePath + "json/" + fileName + ".json";

    std::ofstream file(jsonFile);

    if (file.is_open()) {
        file << data.to_json();; // Write JSON string to file
        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << fileName << ".json" << std::endl;
    }
}

void writeVprData(const std::string &basePath, const std::string &fileName, const ReportData &data) {
    std::string placeFile = basePath + "place/" + fileName + ".place";
    std::string netFile = basePath + "net/" + fileName + ".net";

    int k = 3;

    if (fileName.find("_k3") != std::string::npos) {
        k = 3;
    } else if (fileName.find("_k4") != std::string::npos) {
        k = 4;
    } else if (fileName.find("_k5") != std::string::npos) {
        k = 5;
    } else if (fileName.find("_k6") != std::string::npos) {
        k = 6;
    }

    std::ofstream file(netFile);
    if (file.is_open()) {
        for (int node = 0; node < nNodes; node++) {
            int inDegree = nPredV[node];
            int outDegree = nSuccV[node];
            if (outDegree == 0) {
                for (int pre = 0; pre < nNodes; pre++) {
                    if (predecessors[node][pre]) {
                        file << ".output out_" << node << ":" << pre << std::endl;
                        file << "pinlist: " << pre << std::endl << std::endl;
                    }
                }
            } else if (inDegree == 0) {
                file << ".input " << node << std::endl;
                file << "pinlist: " << node << std::endl << std::endl;
            } else {
                file << ".clb " << node << "   # Only LUT used." << std::endl;
                file << "pinlist:";
                int counter = 0;
                for (int pre = 0; pre < nNodes; pre++) {
                    if (predecessors[node][pre]) {
                        file << " " << pre;
                        counter++;
                    }
                }
                for (int i = 0; i < k - counter; i++)
                    file << " open";
                file << " " << node;
                file << " open" << std::endl;

                file << "subblock: " << node;
                for (int i = 0; i < counter; i++) {
                    file << " " << i;
                }
                for (int i = 0; i < k - counter; i++)
                    file << " open";
                file << " " << k << " open" << std::endl << std::endl;
            }
        }
        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << fileName << ".json" << std::endl;
    }
    file = std::ofstream(placeFile);
    if (file.is_open()) {
        file << "Netlist file: " << fileName << " Architecture file: k" << k << "-n1.xml" << std::endl;
        file << "Array size: " << nCellsSqrt - 2 << " x " << nCellsSqrt - 2 << " logic blocks " << std::endl;
        file << "#block name\tX\tY\tsubblk\tblock_number\n" << std::endl;
        file << "#----------\t--\t--\t------\t------------" << std::endl;

        int counter = 0;
        for (int node = 0; node < nNodes; node++) {
            int cell = data.n2c[node];
            int place = data.placement[cell];
            if (place > -1) {
                int l = cell / nCellsSqrt;
                int c = cell % nCellsSqrt;
                if (nSuccV[node] == 0) {
                    for (int pre = 0; pre < nNodes; pre++) {
                        if (predecessors[node][pre]) {
                            file << "out_" << node << ":" << pre << "\t" << c << "\t" << l << "\t" << 0 << "\t#" <<
                                    counter << std::endl;
                        }
                    }
                } else {
                    file << node << "\t" << c << "\t" << l << "\t" << 0 << "\t#" << counter << std::endl;
                }
            }
            counter++;
        }

        file.close();
    } else {
        std::cerr << "Error opening file for writing: " << fileName << ".json" << std::endl;
    }
}
