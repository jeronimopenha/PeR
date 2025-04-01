#include <fpga/fpgaUtil.h>

FpgaReportData::FpgaReportData()
    : _time(0), cacheMisses(0), tries(0), swaps(0), totalCost(0) {
}

// Constructor for easy initialization
FpgaReportData::FpgaReportData(float _time, const string &dot_name, const string &dot_path,
                               const string &placer, int cacheMisses, int tries, int swaps,
                               const string &edges_algorithm,
                               int total_cost, const vector<int> &placement, const vector<int> &n2c)
    : _time(_time), dotName(dot_name), dotPath(dot_path), placer(placer), cacheMisses(cacheMisses), tries(tries),
      swaps(swaps), edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c) {
}

// Serialize ReportData to a JSON string
string FpgaReportData::to_json() const {
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

void fpgaSavePlacedDot(vector<int> n2c, vector<pair<int, int> > ed, int nCellsSqrt,
                       const string &filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error!" << endl;
        return;
    }

    vector<int> cells(nCellsSqrt * nCellsSqrt, -1);

    for (int i = 0; i < n2c.size(); i++) {
        if (n2c[i] > -1)
            cells[n2c[i]] = i;
    }

    // write the dot header
    file << "digraph layout{" << endl;
    file << "rankdir=TB; \n" << endl;
    file << "splines=ortho; \n" << endl;
    file << "node [style=filled shape=square fixedsize=true width=0.6];" << endl;

    for (int i = 0; i < cells.size(); i++) {
        if (cells[i] == -1) {
            file << i << "[label=\"\", fontsize=8, fillcolor=\"#ffffff\"];" << endl;
        } else {
            file << i << "[label=\"" << cells[i] << "\", fontsize=8, fillcolor=\"#a9ccde\"];" << endl;
        }
    }
    file << "edge [constraint=false, style=vis];" << endl;
    //normal edges
    for (auto [fst,snd]: ed) {
        file << n2c[fst] << " -> " << n2c[snd] << ";" << endl;
    }


    file << "edge [constraint=true, style=invis];" << endl;
    //structural edges
    for (int j = 0; j < nCellsSqrt; j++) {
        for (int i = 0; i < nCellsSqrt; i++) {
            int c = j + i * nCellsSqrt;
            if (i == nCellsSqrt - 1) {
                file << c << ";" << endl;
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
        file << "};" << endl;
    }


    // write the dot footer
    file << "}" << endl;
    file.close();
}


vector<vector<int> > fpgaGetAdjCellsDist(const int nCellsSqrt) {
    const int max_dist = (nCellsSqrt - 1) * 2;
    vector<vector<int> > meshDistances;
    vector<vector<vector<int> > > distance_table_raw(max_dist);
    for (int l = 0; l < nCellsSqrt; ++l) {
        for (int c = 0; c < nCellsSqrt; ++c) {
            if (l == 0 && c == 0) continue; // Skip t
            const int dist = l + c;

            // Lambda to check if a coordinate pair is already in a list
            auto contains = [](const vector<vector<int> > &vec, const vector<int> &pair) {
                return find(vec.begin(), vec.end(), pair) != vec.end();
            };

            // Add unique coordinates to the distance table
            if (!contains(distance_table_raw[dist - 1], {l, c})) {
                distance_table_raw[dist - 1].push_back({l, c});
            }
            if (!contains(distance_table_raw[dist - 1], {l, -c})) {
                distance_table_raw[dist - 1].push_back({l, -c});
            }
            if (!contains(distance_table_raw[dist - 1], {-l, -c})) {
                distance_table_raw[dist - 1].push_back({-l, -c});
            }
            if (!contains(distance_table_raw[dist - 1], {-l, c})) {
                distance_table_raw[dist - 1].push_back({-l, c});
            }
        }
    }
    // Shuffle the distance table if make_shuffle is set

    auto rng = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    for (auto &d: distance_table_raw) {
        shuffle(d.begin(), d.end(), rng);
        for (const auto &pair: d) {
            meshDistances.push_back(pair);
        }
    }

    return meshDistances;
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

int fpgaMinBorderDist(const int cell, const int nCellsSqrt) {
    const int line = cell / nCellsSqrt;
    const int column = cell % nCellsSqrt;

    int d_top = line;
    int d_bottom = nCellsSqrt - 1 - line;
    int d_left = column;
    int d_right = nCellsSqrt - 1 - column;

    return std::min({d_top, d_bottom, d_left, d_right});
}

void writeJson(const string &basePath,
               const string &reportPath,
               const string &algPath,
               const string &fileName,
               const FpgaReportData &data) {
    string finalPath = basePath + reportPath + algPath + "/json/";
    string jsonFile = finalPath + fileName + ".json";

    createDir(finalPath);

    ofstream file(jsonFile);

    if (file.is_open()) {
        file << data.to_json();; // Write JSON string to file
        file.close();
    } else {
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
    }
}

void writeVprData(const string &basePath,
                  const string &reportPath,
                  const string &algPath,
                  const string &fileName,
                  const FpgaReportData &data,
                  FPGAGraph g) {
    string placePath = basePath + reportPath + algPath + "/place/";
    string placeFile = placePath + fileName + ".place";

    string netBasePath = reportPath + algPath + "/net/";
    string netPath = basePath + netBasePath;
    string netFile = netPath + fileName + ".net";

    createDir(placePath);
    createDir(netPath);

    int k = 3;

    if (fileName.find("_k3") != string::npos) {
        k = 3;
    } else if (fileName.find("_k4") != string::npos) {
        k = 4;
    } else if (fileName.find("_k5") != string::npos) {
        k = 5;
    } else if (fileName.find("_k6") != string::npos) {
        k = 6;
    }

    ofstream file(netFile);
    if (file.is_open()) {
        for (int node = 0; node < g.nNodes; node++) {
            int inDegree = g.nPredV[node];
            int outDegree = g.nSuccV[node];
            if (outDegree == 0) {
                for (int pre = 0; pre < g.nNodes; pre++) {
                    if (g.predecessors[node][pre]) {
                        file << ".output out_" << node << ":" << pre << endl;
                        file << "pinlist: " << pre << endl << endl;
                    }
                }
            } else if (inDegree == 0) {
                file << ".input " << node << endl;
                file << "pinlist: " << node << endl << endl;
            } else {
                file << ".clb " << node << "   # Only LUT used." << endl;
                file << "pinlist:";
                int counter = 0;
                for (int pre = 0; pre < g.nNodes; pre++) {
                    if (g.predecessors[node][pre]) {
                        file << " " << pre;
                        counter++;
                    }
                }
                for (int i = 0; i < k - counter; i++)
                    file << " open";
                file << " " << node;
                file << " open" << endl;

                file << "subblock: " << node;
                for (int i = 0; i < counter; i++) {
                    file << " " << i;
                }
                for (int i = 0; i < k - counter; i++)
                    file << " open";
                file << " " << k << " open" << endl << endl;
            }
        }
        file.close();
    } else {
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
    }
    file = ofstream(placeFile);
    if (file.is_open()) {
        file << "Netlist file: " + netBasePath << fileName << ".net Architecture file: arch/k" << k <<
                "-n1.xml" << endl;

        file << "Array size: " << g.nCellsSqrt - 2 << " x " << g.nCellsSqrt - 2 << " logic blocks " << endl;
        file << "#block name\tX\tY\tsubblk\tblock_number\n" << endl;
        file << "#----------\t--\t--\t------\t------------" << endl;

        int counter = 0;
        for (int node = 0; node < g.nNodes; node++) {
            int cell = data.n2c[node];
            int place = data.placement[cell];
            if (place > -1) {
                int l = cell / g.nCellsSqrt;
                int c = cell % g.nCellsSqrt;
                if (g.nSuccV[node] == 0) {
                    for (int pre = 0; pre < g.nNodes; pre++) {
                        if (g.predecessors[node][pre]) {
                            file << "out_" << node << ":" << pre << "\t" << c << "\t" << l << "\t" << 0 << "\t#" <<
                                    counter << endl;
                        }
                    }
                } else {
                    file << node << "\t" << c << "\t" << l << "\t" << 0 << "\t#" << counter << endl;
                }
            }
            counter++;
        }

        file.close();
    } else {
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
    }
}
bool isInvalidCell(const int cell, const int nCellsSqrt)
{
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

bool isIOCell(const int cell, const int nCellsSqrt)
{
    const int l = cell / nCellsSqrt;
    const int c = cell % nCellsSqrt;
    return l == 0 || l == nCellsSqrt - 1 || c == 0 || c == nCellsSqrt - 1;
}