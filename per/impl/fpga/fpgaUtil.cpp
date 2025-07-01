#include <fpga/fpgaUtil.h>
#include <utility>

using namespace std;

FpgaReportData::FpgaReportData() = default;

// Constructor for easy initialization
FpgaReportData::FpgaReportData(const double _time, string dotName, string dotPath, string placer,
                               const long cacheMisses, const long w, const long wCost, const long cachePenalties,
                               const long clbTries, const long ioTries, const long tries, const long triesP,
                               const long swaps, string edges_algorithm, const long totalCost, const long lPCost,
                               const vector<long> &placement, const vector<long> &n2c)
    : _time(_time),
      dotName(std::move(dotName)),
      dotPath(std::move(dotPath)),
      placer(std::move(placer)),
      cacheMisses(cacheMisses),
      w(w),
      wCost(wCost),
      cachePenalties(cachePenalties),
      clbTries(clbTries),
      ioTries(ioTries),
      tries(tries),
      triesP(triesP),
      swaps(swaps),
      edgesAlgorithm(std::move(edges_algorithm)),
      totalCost(totalCost),
      lPCost(lPCost),
      placement(placement),
      n2c(n2c) {
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
            << "  \"w\": " << w << ",\n"
            << "  \"wCost\": " << wCost << ",\n"
            << "  \"cachePenalties\": " << cachePenalties << ",\n"
            << "  \"clbTries\": " << clbTries << ",\n"
            << "  \"ioTries\": " << ioTries << ",\n"
            << "  \"tries\": " << tries << ",\n"
            << "  \"triesP\": " << triesP << ",\n"
            << "  \"swaps\": " << swaps << ",\n"
            << "  \"edgesAlgorithm\": \"" << edgesAlgorithm << "\",\n"
            << "  \"totalCost\": " << totalCost << ",\n"
            << "  \"lPCost\": " << lPCost << "\n";
    /*<< "  \"placement\": [";

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
oss << "]\n";*/

    oss << "}\n";
    return oss.str();
}

void fpgaSavePlacedDot(vector<long> &n2c, const vector<pair<long, long> > &ed, const long nCellsSqrt,
                       const string &filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error!" << endl;
        return;
    }

    vector<long> cells(nCellsSqrt * nCellsSqrt, -1);

    for (long i = 0; i < static_cast<long>(n2c.size()); i++) {
        if (n2c[i] > -1)
            cells[n2c[i]] = i;
    }

    // write the dot header
    file << "digraph layout{" << endl;
    file << "rankdir=TB; \n" << endl;
    file << "splines=ortho; \n" << endl;
    file << "node [style=filled shape=square fixedsize=true width=0.6];" << endl;

    for (long i = 0; i < cells.size(); i++) {
        if (cells[i] == -1) {
            file << i << "[label=\"\", fontsize=8, fillcolor=\"#ffffff\"];" << endl;
        } else {
            file << i << "[label=\"" << cells[i] << "\", fontsize=8, fillcolor=\"#a9ccde\"];" << endl;
        }
    }
    file << "edge [constraint=false, style=vis];" << endl;
    //normal edges
    for (auto &[fst,snd]: ed) {
        if (n2c[fst] != -1 && n2c[snd] != -1)
            file << n2c[fst] << " -> " << n2c[snd] << ";" << endl;
    }


    file << "edge [constraint=true, style=invis];" << endl;
    //structural edges
    for (long j = 0; j < nCellsSqrt; j++) {
        for (long i = 0; i < nCellsSqrt; i++) {
            long c = j + i * nCellsSqrt;
            if (i == nCellsSqrt - 1) {
                file << c << ";" << endl;
            } else {
                file << c << " -> ";
            }
        }
    }

    for (long i = 0; i < nCellsSqrt; i++) {
        file << "rank = same { ";
        for (long j = 0; j < nCellsSqrt; j++) {
            long c = i * nCellsSqrt + j;
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


vector<vector<long> > fpgaGetAdjCellsDist(const long nCellsSqrt) {
    const long max_dist = (nCellsSqrt * 2) - 1;
    vector<vector<long> > meshDistances;
    vector<vector<vector<long> > > distance_table_raw(max_dist);
    for (long l = 0; l < nCellsSqrt; ++l) {
        for (long c = 0; c < nCellsSqrt; ++c) {
            if (l == 0 && c == 0)
                continue; // Skip t

            const long dist = l + c;

            // Lambda to check if a coordinate pair is already in a list
            auto contains = [](const vector<vector<long> > &vec, const vector<long> &pair) {
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


long fpgaCalcGraphTotalDistance(const vector<long> &n2c, const vector<pair<long, long> > &edges,
                                const long nCellsSqrt) {
    long totalDist = -static_cast<long>(edges.size());

    for (const auto &[fst, snd]: edges) {
        const long tempDist = getManhattanDist(n2c[fst], n2c[snd], nCellsSqrt);

        // Acc the total distance
        totalDist += tempDist;
    }

    return totalDist;
}

long fpgaCalcGraphLPDistance(const vector<long> &longestPath, const vector<long> &n2c, const long nCellsSqrt) {
    long totalDist = 0;

    for (long idx = 0; idx < longestPath.size() - 1; idx++) {
        const long tempDist = getManhattanDist(n2c[longestPath[idx]], n2c[longestPath[idx + 1]], nCellsSqrt);

        // Acc the total distance
        totalDist += tempDist;
    }

    return totalDist;
}

long fpgaMinBorderDist(const long cell, const long nCellsSqrt) {
    const long line = cell / nCellsSqrt;
    const long column = cell % nCellsSqrt;

    long d_top = line;
    long d_bottom = nCellsSqrt - 1 - line;
    long d_left = column;
    long d_right = nCellsSqrt - 1 - column;

    return std::min({d_top, d_bottom, d_left, d_right});
}

void fpgaWriteJson(const string &basePath,
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

void fpgaWriteVprData(const string &basePath,
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

    long k = 3;

    if (fileName.find("_k3") != string::npos)
        k = 3;

    else if (fileName.find("_k4") != string::npos)
        k = 4;

    else if (fileName.find("_k5") != string::npos)
        k = 5;

    else if (fileName.find("_k6") != string::npos)
        k = 6;


    ofstream file(netFile);
    if (file.is_open()) {
        for (long node = 0; node < g.nNodes; node++) {
            const long inDegree = g.nPredV[node];
            const long outDegree = g.nSuccV[node];
            if (outDegree == 0) {
                for (long pre = 0; pre < g.nNodes; pre++) {
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
                long counter = 0;
                for (long pre = 0; pre < g.nNodes; pre++) {
                    if (g.predecessors[node][pre]) {
                        file << " " << pre;
                        counter++;
                    }
                }
                for (long i = 0; i < k - counter; i++)
                    file << " open";
                file << " " << node;
                file << " open" << endl;

                file << "subblock: " << node;
                for (long i = 0; i < counter; i++)
                    file << " " << i;

                for (long i = 0; i < k - counter; i++)
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

        long counter = 0;
        for (long node = 0; node < g.nNodes; node++) {
            const long cell = data.n2c[node];
            const long place = data.placement[cell];
            if (place > -1) {
                long l = cell / g.nCellsSqrt;
                long c = cell % g.nCellsSqrt;
                if (g.nSuccV[node] == 0) {
                    for (long pre = 0; pre < g.nNodes; pre++) {
                        if (g.predecessors[node][pre]) {
                            file << "out_" << node << ":" << pre << "\t" << c << "\t" << l << "\t" << 0 << "\t#" <<
                                    counter << endl;
                        }
                    }
                } else
                    file << node << "\t" << c << "\t" << l << "\t" << 0 << "\t#" << counter << endl;
            }
            counter++;
        }

        file.close();
    } else
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
}

bool fpgaIsInvalidCell(const long cell, const long nCellsSqrt) {
    const long l = cell / nCellsSqrt;
    const long c = cell % nCellsSqrt;

    const bool outOfBounds = (l < 0 || l >= nCellsSqrt || c < 0 || c >= nCellsSqrt);

    const bool isCorner =
            (l == 0 && c == 0) ||
            (l == 0 && c == nCellsSqrt - 1) ||
            (l == nCellsSqrt - 1 && c == 0) ||
            (l == nCellsSqrt - 1 && c == nCellsSqrt - 1);

    return outOfBounds || isCorner;
}

bool fpgaIsIOCell(const long cell, const long nCellsSqrt) {
    const long l = cell / nCellsSqrt;
    const long c = cell % nCellsSqrt;
    return l == 0 || l == nCellsSqrt - 1 || c == 0 || c == nCellsSqrt - 1;
}
