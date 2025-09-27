#include <common/util.h>
#include <fpga/fpgaUtil.h>
#include <common/std_image_write.h>

#include <utility>


using namespace std;

/**
 * Basic report data
 */
FpgaReportData::FpgaReportData() = default;

/**
 * Constructor for easy reportData initialization
 */
FpgaReportData::FpgaReportData(
    const double _time,
    string dotName,
    string dotPath,
    string placer,
    const long size,
    const long nNodes,
    const long nIOs,
    string edgesAlgorithm,
    string costStrategy,
    const long totalCost,
    const vector<vector<long> > &c2n,
    const vector<pair<long, long> > &n2c
#ifdef MAKE_METRICS
    ,
#ifdef USE_CACHE
    const long cacheMisses,
    const long w,
    const long wCost,
    const long cachePenalties,
#endif
    const long clbTries,
    const long ioTries,
    const long tries,
#ifdef USE_CACHE
    const long triesP,
#endif
    const long triesPerNode,
    const long swaps,
    vector<map<long, long> > hist,
    vector<long> heatEnd,
    vector<long> heatBegin,
    map<long, vector<long> > orDest
#endif
)
    : _time(_time),
      dotName(move(dotName)),
      dotPath(move(dotPath)),
      placer(move(placer)),
      size(size),
      nNodes(nNodes),
      nIOs(nIOs),
      edgesAlgorithm(move(edgesAlgorithm)),
      costStrategy(move(costStrategy)),
      totalCost(totalCost),
      c2n(c2n),
      n2c(n2c)
#ifdef MAKE_METRICS
      ,
#ifdef USE_CACHE
cacheMisses(cacheMisses),
        w(w),
        wCost(wCost),
        cachePenalties(cachePenalties),
#endif
clbTries(clbTries),
        ioTries(ioTries),
        tries(tries),
#ifdef USE_CACHE
triesP(triesP),
#endif
triesPerNode(triesPerNode),
        swaps(swaps),
        hist(move(hist)),
        heatEnd(move(heatEnd)),
        heatBegin(move(heatBegin)),
        orDest(move(orDest))
#endif
{
}


/**
 * Serialize ReportData to a JSON string
 * @return
 */
string FpgaReportData::to_json() const {
    ostringstream oss;
    oss << "{\n"
            << "  \"time\": " << _time << ",\n"
            << "  \"dotName\": \"" << dotName << "\",\n"
            << "  \"dotPath\": \"" << dotPath << "\",\n"
            << "  \"placer\": \"" << placer << "\",\n"
            << "  \"size\": \"" << size << "x" << size << "\",\n"
            << "  \"border\": " << size << ",\n"
            << "  \"nNodes\": " << nNodes << ",\n"
            << "  \"nIOs\": " << nIOs << ",\n"
            << "  \"nIOpCell\": " << nIOpCell << ",\n"
            << "  \"edgesAlgorithm\": \"" << edgesAlgorithm << "\",\n"
            << "  \"costStrategy\": \"" << costStrategy << "\",\n"
            << "  \"totalCost\": " << totalCost << ",\n"
#ifdef MAKE_METRICS
#ifdef CACHE
            << "  \"cacheMisses\": " << cacheMisses << ",\n"
               << "  \"w\": " << w << ",\n"
               << "  \"wCost\": " << wCost << ",\n"
               << "  \"cachePenalties\": " << cachePenalties << ",\n"
#endif
            << "  \"clbTries\": " << clbTries << ",\n"
               << "  \"ioTries\": " << ioTries << ",\n"
#ifdef CACHE
            << "  \"tries\": " << triesP << ",\n"
#else
            << "  \"tries\": " << tries << ",\n"
#endif
            << "  \"triesPerNode\": " << triesPerNode << ",\n"
               << "  \"swaps\": " << swaps << ",\n"
#endif
            ;
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


/**
 * This function takes the distVectors offsets to find a free cell by YOTO and YOTT algorithms
 * @param nCellsSqrt
 * @return
 */
vector<vector<long> > fpgaGetDistVectors(const long nCellsSqrt) {
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
                bool found = find(vec.begin(), vec.end(), pair) != vec.end();
                return found;
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
    // Shuffle the distance table if you make_shuffle is set

    auto rng = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    for (auto &d: distance_table_raw) {
        shuffle(d.begin(), d.end(), rng);
        for (const auto &pair: d) {
            meshDistances.push_back(pair);
        }
    }

    return meshDistances;
}

#ifdef MAKE_METRICS
/**
 * Calculates the total distance cost with manhattan method
 * @param n2c
 * @param edges
 * @param nCellsSqrt
 * @return
 */
long fpgaCalcGraphTotalDistance(const vector<pair<long, long> > &n2c, const vector<pair<long, long> > &edges,
                                const long nCellsSqrt) {
    long totalDist = -static_cast<long>(edges.size());

    for (const auto &[fst, snd]: edges) {
        const long tempDist = getManhattanDist(n2c[fst].first, n2c[snd].first, nCellsSqrt);

        // Acc the total distance
        totalDist += tempDist;
    }

    return totalDist;
}
#endif

/*long fpgaCalcGraphLPDistance(const vector<long> &longestPath, const vector<long> &n2c, const long nCellsSqrt) {
    long totalDist = 0;

    for (long idx = 0; idx < longestPath.size() - 1; idx++) {
        const long tempDist = getManhattanDist(n2c[longestPath[idx]], n2c[longestPath[idx + 1]], nCellsSqrt);

        // Acc the total distance
        totalDist += tempDist;
    }

    return totalDist;
}*/

/**
 * Finds wich border is near to the node
 * @param cell
 * @param nCellsSqrt
 * @return
 */
long fpgaMinBorderDist(const long cell, const long nCellsSqrt) {
    const long line = getLine(cell, nCellsSqrt);
    const long column = getColumn(cell, nCellsSqrt);

    long d_top = line;
    long d_bottom = nCellsSqrt - 1 - line;
    long d_left = column;
    long d_right = nCellsSqrt - 1 - column;

    return min({d_top, d_bottom, d_left, d_right});
}


/**
 * Writes the json reports (global and metrics)
 * @param basePath
 * @param fileName
 * @param data
 */
void fpgaWriteReports(const string &basePath,
                      const string &fileName,
                      const FpgaReportData &data) {
    string reportFullPath = basePath + reportPath + algPath + "/json/";
    string reportFile = reportFullPath + fileName + ".json";

    createDir(reportFullPath);

    ofstream reportWriter(reportFile);

    if (reportWriter.is_open()) {
        reportWriter << data.to_json();
        reportWriter.close();
    } else {
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
    }

#ifdef MAKE_METRICS
    reportFullPath = basePath + reportPath + algPath + "/metrics/";
    reportFile = reportFullPath + fileName + ".json";

    createDir(reportFullPath);

    reportWriter = ofstream(reportFile);

    if (reportWriter.is_open()) {
        reportWriter << data.metrics_to_json();
        reportWriter.close();
    } else {
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
    }
#endif
}

/**
 * Writes VPR9 place files to be used by VPR9 to route
 * @param basePath
 * @param fileName
 * @param data
 * @param g
 */
void fpgaWriteVpr9Data(const string &basePath,
                       const string &fileName,
                       const FpgaReportData &data,
                       FPGAGraph g) {
    string placePath = basePath + reportPath + algPath + "/place/";
    string placeFile = placePath + fileName + ".place";

    createDir(placePath);

    ofstream file = ofstream(placeFile);
    if (file.is_open()) {
        //Netlist_File: ctrl_k.net Netlist_ID: SHA256:
        file << "Netlist_File: " << fileName.substr(0, fileName.size() - 2) << ".net Netlist_ID: SHA256:" << endl;

        file << "Array size: " << g.nCellsSqrt << " x " << g.nCellsSqrt << " logic blocks " << endl << endl;
        file << "#block name\tx\ty\tsubblk\tlayer\tblock number" << endl;
        file << "#----------\t--\t--\t------\t------------" << endl;

        long counter = 0;
        for (long node = 0; node < g.nNodes; node++) {
            const long cell = data.n2c[node].first;
            const long subblock = data.n2c[node].second;
            const long place = data.c2n[cell][subblock];
            if (place > -1) {
                long l = cell / g.nCellsSqrt;
                long c = cell % g.nCellsSqrt;
                if (g.nSuccV[node] == 0)
                    //for (long pre = 0; pre < g.nNodes; pre++) {
                    //    if (g.predecessors[node][pre]) {
                    file << "out:" << node << "\t" << c << "\t" << l << "\t" << subblock << "\t" << 0 << "\t#" <<
                            counter << endl;
                    //    }
                    //}
                else
                    file << node << "\t" << c << "\t" << l << "\t" << subblock << "\t" << 0 << "\t#" <<
                            counter << endl;
            }
            counter++;
        }

        file.close();
    } else
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
}

/**
 * Writes VPR9 place files to be used by VPR5 to route
 * @param basePath
 * @param fileName
 * @param data
 * @param g
 */
void fpgaWriteVpr5Data(const string &basePath,
                       const string &fileName,
                       const FpgaReportData &data,
                       FPGAGraph g) {
    string placePath = basePath + reportPath + algPath + "/place/";
    string placeFile = placePath + fileName + ".place";

    string netBasePath = reportPath + algPath + "/blif/";
    string blifPath = basePath + netBasePath;
    string blifFile = blifPath + fileName + ".blif";

    createDir(placePath);
    //createDir(blifPath);

    long k = 6;

    /*if (fileName.find("_k3") != string::npos)
        k = 3;

    else if (fileName.find("_k4") != string::npos)
        k = 4;

    else if (fileName.find("_k5") != string::npos)
        k = 5;

    else if (fileName.find("_k6") != string::npos)
        k = 6;*/


    ofstream file(blifFile);
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

    //fixme
    /*file = ofstream(placeFile);
    if (file.is_open()) {
        file << "Netlist file: " + netBasePath << fileName << ".net Architecture file: arch/k" << k <<
                "-n1.xml" << endl;

        file << "Array size: " << g.nCellsSqrt - 2 << " x " << g.nCellsSqrt - 2 << " logic blocks " << endl;
        file << "#block name\tX\tY\tsubblk\tblock_number\n" << endl;
        file << "#----------\t--\t--\t------\t------------" << endl;

        long counter = 0;
        for (long node = 0; node < g.nNodes; node++) {
            const long cell = data.n2c[node];
            const long place = data.c2n[cell];
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
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;*/
}

/**
 * Verifies if the cell is inside the matrix or not
 * @param line
 * @param column
 * @param nCellsSqrt
 * @return
 */
bool fpgaIsInvalidCell(const long line, const long column, const long nCellsSqrt) {
    const bool outOfBounds = (line < 0 || line >= nCellsSqrt || column < 0 || column >= nCellsSqrt);

    const bool isCorner =
            (line == 0 && column == 0) ||
            (line == 0 && column == nCellsSqrt - 1) ||
            (line == nCellsSqrt - 1 && column == 0) ||
            (line == nCellsSqrt - 1 && column == nCellsSqrt - 1);

    return outOfBounds || isCorner;
}

/**
 * Verifies of the cell is an IO cell (border cell in this case)
 * @param line
 * @param column
 * @param nCellsSqrt
 * @return
 */
bool fpgaIsIOCell(const long line, const long column, const long nCellsSqrt) {
    return line == 0 || line == nCellsSqrt - 1 || column == 0 || column == nCellsSqrt - 1;
}

#if defined(SCAN_STRATEGY) || defined(LIMIT_STRATEGY)
long getQuadrant(const long line, const long column, const long nCellsSqrt) {
    constexpr long nQuadrants = QUADRANTS;

    const long nQuadrantsSqrt = ceil(sqrt(nQuadrants));

    const long quadSize = (nCellsSqrt + nQuadrantsSqrt - 1) / nQuadrantsSqrt;

    const long quad_c = column / quadSize;
    const long quad_l = line / quadSize;

    return quad_l * 4 + quad_c;
}


/**
 * Returns the top, bottom, left and right quadrant indexes
 * @param q
 * @return
 */
vector<pair<long, int> > getAdjacentQuadrants(const long q) {
    vector<pair<long, int> > adj;

    constexpr long nQuadrants = QUADRANTS;
    const long nQuadrantsSqrt = ceil(sqrt(nQuadrants));


    const long line = q / nQuadrantsSqrt;
    const long column = q % nQuadrantsSqrt;

    //fixme transform the direction number in an enum
    // top
    if (line > 0) adj.emplace_back((line - 1) * nQuadrantsSqrt + column, 0);
    // bottom
    if (line < nQuadrantsSqrt - 1) adj.emplace_back((line + 1) * nQuadrantsSqrt + column, 1);
    // left
    if (column > 0) adj.emplace_back(line * nQuadrantsSqrt + (column - 1), 2);
    // right
    if (column < nQuadrantsSqrt - 1) adj.emplace_back(line * nQuadrantsSqrt + (column + 1), 3);

    return adj;
}
#endif

/**
 * Function to map normalized value (0 to 1) to simple RGB (blue to red)
 * @param normValue
 * @return
 */
RGB valueToRGB(const float normValue) {
    struct ColorPoint {
        float position; // between 0.0 e 1.0
        int r, g, b;
    };

    const vector<ColorPoint> colors = {
        //{0.0f, 255, 255, 255}, // white
        {0.0f, 173, 216, 230}, //lightblue
        {0.33f, 100, 149, 237}, // cornflower blue
        {0.66f, 138, 43, 226}, // blueviolet
        {1.0f, 255, 0, 0} // red
    };

    // limit value entre 0 e 1
    //value = max(0.0f, min(1.0f, value));

    // find interpolar interval
    for (size_t i = 1; i < colors.size(); ++i) {
        if (normValue <= colors[i].position) {
            const auto &[position0, r0, g0, b0] = colors[i - 1];
            const auto &[position1, r1, g1, b1] = colors[i];

            const float range = position1 - position0;
            const float localVal = (normValue - position0) / range;

            const int r = r0 + static_cast<int>(static_cast<float>(r1 - r0) * localVal);
            const int g = g0 + static_cast<int>(static_cast<float>((g1 - g0)) * localVal);
            const int b = b0 + static_cast<int>(static_cast<float>((b1 - b0)) * localVal);

            return {r, g, b};
        }
    }

    // fallback (should never occur)
    return {255, 255, 255};
}


#ifdef MAKE_METRICS
/**
 * Serialize metrics to a JSON string
 * @return
 */
string FpgaReportData::metrics_to_json() const {
    ostringstream oss;
    oss << "{\n";

    // heatBegin
    oss << "  \"heatBegin\": [";
    for (size_t i = 0; i < heatBegin.size(); ++i) {
        oss << heatBegin[i];
        if (i != heatBegin.size() - 1) oss << ", ";
    }
    oss << "],\n";

    // heatEnd
    oss << "  \"heatEnd\": [";
    for (size_t i = 0; i < heatEnd.size(); ++i) {
        oss << heatEnd[i];
        if (i != heatEnd.size() - 1) oss << ", ";
    }
    oss << "],\n";

    // Histograms
    oss << "  \"hist\": {\n";
    for (size_t h = 0; h < hist.size(); ++h) {
        oss << "    \"" << h << "\": {";
        const auto &map = hist[h];
        size_t count = 0;
        for (const auto &[k, v]: map) {
            oss << "\"" << k << "\": " << v;
            if (++count != map.size()) oss << ", ";
        }
        oss << "}";
        if (h != hist.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << "  },\n";

    // orDest
    oss << "  \"orDest\": {\n";
    size_t count = 0;
    for (const auto &[k, v]: orDest) {
        oss << " \"" << k << "\": [";
        size_t vCount = 0;
        for (const auto dest: v) {
            oss << dest;
            if (++vCount != v.size()) oss << ", ";
        }
        oss << "]";
        if (++count != orDest.size()) oss << ", ";
        oss << "\n";
    }
    oss << "}";
    oss << "\n";

    oss << "}\n";


    return oss.str();
}
#endif

#ifdef PRINT_DOT

/**
 * Saves the dot placement file to debug.
 * @param n2c
 * @param c2n
 * @param ed
 * @param nCellsSqrt
 */
void fpgaSavePlacedDot(vector<pair<long, long> > &n2c, vector<vector<long> > &c2n, const vector<pair<long, long> > &ed,
                       const long nCellsSqrt) {
    string fileString;

    // write the dot header
    fileString += "digraph layout{\n";
    fileString += "rankdir=TB; \n\n";
    fileString += "splines=ortho; \n\n";
    fileString += "node [style=filled shape=square fixedsize=true width=0.6];\n";

    for (long i = 0; i < c2n.size(); i++) {
        string label = "[label = \"";
        bool flag = !c2n[i].empty();

        for (long j = 0; j < c2n[i].size(); j++) {
            if (c2n[i][j] == -1) {
                break;
            }
            flag = true;
            if (j > 0)
                label += ",\n";
            label += to_string(c2n[i][j]);
        }
        fileString += to_string(i) + label + "\", fontsize=8, fillcolor=";

        if (flag)
            fileString += "\"#a9ccde\"];";
        else
            fileString += "\"#ffffff\"];";

        fileString += "\n";
    }
    fileString += "edge [constraint=false, style=vis];\n";
    //normal edges
    for (auto &[fst,snd]: ed) {
        if (n2c[fst].first != -1 && n2c[snd].first != -1)
            fileString += to_string(n2c[fst].first) + " -> " + to_string(n2c[snd].first) + ";\n";
    }


    fileString += "edge [constraint=true, style=invis];\n";
    //structural edges
    for (long j = 0; j < nCellsSqrt; j++) {
        for (long i = 0; i < nCellsSqrt; i++) {
            long c = j + i * nCellsSqrt;
            if (i == nCellsSqrt - 1) {
                fileString += to_string(c) + ";\n";
            } else {
                fileString += to_string(c) + " -> ";
            }
        }
    }

    for (long i = 0; i < nCellsSqrt; i++) {
        fileString += "rank = same { ";
        for (long j = 0; j < nCellsSqrt; j++) {
            long c = i * nCellsSqrt + j;
            if (j == nCellsSqrt - 1) {
                fileString += to_string(c) + ";";
            } else {
                fileString += to_string(c) + " -> ";
            }
        }
        fileString += "};\n";
    }

    // write the dot footer
    fileString += "}\n";

    const string filePath = DOT_PATH;

    ofstream file(filePath);
    if (!file) {
        cerr << "Error!" << endl;
        return;
    }
    file << fileString;
    file.close();
}
#endif

#ifdef PRINT_IMG
void writeMap(const vector<vector<long> > &c2n, const pair<long, long> &lastPlaced, const long nCellsSqrt) {
    const string filePath = JPG_PATH;
    constexpr long minImageSize = 1000;

    const long cellSize = (minImageSize + nCellsSqrt - 1) / nCellsSqrt; // ceil(minImageSize / n)
    const long imageCoreSize = cellSize * nCellsSqrt;

    constexpr long borderPadding = 10;
    const long imageWidth = imageCoreSize + 2 * borderPadding;
    const long imageHeight = imageCoreSize + 2 * borderPadding;

    vector<unsigned char> imageData(imageWidth * imageHeight * 3, 255);

    for (long y = 0; y <= imageCoreSize; y++) {
        for (long x = 0; x <= imageCoreSize; x++) {
            //border is black

            constexpr RGB white{255, 255, 255};
            constexpr RGB black{0, 0, 0};
            constexpr RGB blue{120, 120, 255};
            constexpr RGB red{255, 120, 120};
            constexpr RGB gray{230, 230, 230};

            RGB pixel = white;

            const bool isBorder = (x == 0 || y == 0 || x == imageCoreSize || y == imageCoreSize);

            if (isBorder) {
                pixel = black;
            } else {
                const bool isGridLine = (x % cellSize == 0 || y % cellSize == 0);
                const long srcX = x / cellSize;
                const long srcY = y / cellSize;
                const long cellIdx = srcY * nCellsSqrt + srcX;

                if (isGridLine || fpgaIsInvalidCell(srcY, srcX, nCellsSqrt)) {
                    //grid or invalid cell
                    pixel = white;
                } else {
                    const bool isUsedCell = (static_cast<long>(c2n[cellIdx].size()) > 0l);

                    if (cellIdx == lastPlaced.first) {
                        pixel = blue;
                    } else if (cellIdx == lastPlaced.second) {
                        pixel = red;
                    } else if (isUsedCell) {
                        pixel = gray;
                    }
                }
            }

            //calculate the coords
            const long dstX = x + borderPadding;
            const long dstY = y + borderPadding;
            const long pixelIndex = (dstY * imageWidth + dstX) * 3;

            //apply the values to the pixel
            imageData[pixelIndex + 0] = pixel.r;
            imageData[pixelIndex + 1] = pixel.g;
            imageData[pixelIndex + 2] = pixel.b;
        }
    }

    stbi_write_jpg(filePath.c_str(), static_cast<int>(imageWidth), static_cast<int>(imageHeight), 3,
                   imageData.data(), 100);
}
#endif


/*
/**
 * Writes the heat map for a placement
 * Not used yet
 * @param heatData
 * @param c2n
 * @param nCellsSqrt
 * @param basePath
 * @param fileName
 * @param suffix
 #1#
void writeHeatmap(const vector<long> &heatData,
                  const vector<vector<long> > &c2n,
                  const long nCellsSqrt,
                  const string &basePath,
                  const string &fileName,
                  const string &suffix) {
    const string heatmapPath = basePath + reportPath + algPath + "/heatmap/";
    const string heatmapFile = heatmapPath + fileName + "_" + suffix + ".jpeg";
    createDir(heatmapPath);


    constexpr long minImageSize = 1000;

    const long cellSize = (minImageSize + nCellsSqrt - 1) / nCellsSqrt; // ceil(minImageSize / n)
    const long imageCoreSize = cellSize * nCellsSqrt;

    constexpr long borderPadding = 10;
    const long imageWidth = imageCoreSize + 2 * borderPadding;
    const long imageHeight = imageCoreSize + 2 * borderPadding;

    const long minVal = 0; //*min_element(heatData.begin(), heatData.end());
    const long maxVal = *max_element(heatData.begin(), heatData.end());

    vector<unsigned char> imageData(imageWidth * imageHeight * 3, 255);

    for (long y = 0; y <= imageCoreSize; y++) {
        for (long x = 0; x <= imageCoreSize; x++) {
            //border is black
            RGB pixel{255, 255, 255};

            const bool isBorder = (x == 0 || y == 0 || x == imageCoreSize || y == imageCoreSize);

            if (isBorder) {
                pixel = {0, 0, 0};
            } else {
                const bool isGridLine = (x % cellSize == 0 || y % cellSize == 0);
                const long srcX = x / cellSize;
                const long srcY = y / cellSize;
                const long cellIdx = srcY * nCellsSqrt + srcX;

                if (isGridLine || fpgaIsInvalidCell(srcY, srcX, nCellsSqrt)) {
                    //grid or invalid cell
                    pixel = {255, 255, 255};
                } else {
                    const long val = heatData[cellIdx];
                    const bool isUsedCell = (static_cast<int>(c2n[cellIdx].size()) != -1);

                    if (val != 0) {
                        //heatmap
                        const float normVal = static_cast<float>(val - minVal) / (
                                                  static_cast<float>(maxVal - minVal) + 1e-5f);
                        pixel = valueToRGB(normVal);
                    } else if (isUsedCell) {
                        //gray occupied cell
                        pixel = {224, 224, 224};
                    }
                }
            }

            //calculate the coords
            const long dstX = x + borderPadding;
            const long dstY = y + borderPadding;
            const long pixelIndex = (dstY * imageWidth + dstX) * 3;

            //apply the values to the pixel
            imageData[pixelIndex + 0] = pixel.r;
            imageData[pixelIndex + 1] = pixel.g;
            imageData[pixelIndex + 2] = pixel.b;
        }
    }

    stbi_write_jpg(heatmapFile.c_str(), static_cast<int>(imageWidth), static_cast<int>(imageHeight), 3,
                   imageData.data(), 100);
}*/

/*
void drawChar(vector<unsigned char> &image,
              const int imgWidth,
              const int x,
              const int y,
              const char c,
              const int scale) {
    /*static const map<char, vector<string> > font5x7 = {
        {'0', {" ### ", "#   #", "#  ##", "# # #", "##  #", "#   #", " ### "}},
        {'1', {"  #  ", " ##  ", "# #  ", "  #  ", "  #  ", "  #  ", "#####"}},
        {'2', {" ### ", "#   #", "    #", "   # ", "  #  ", " #   ", "#####"}},
        {'3', {" ### ", "#   #", "    #", " ### ", "    #", "#   #", " ### "}},
        {'4', {"   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # "}},
        {'5', {"#####", "#    ", "#    ", "#### ", "    #", "#   #", " ### "}},
        {'6', {" ### ", "#   #", "#    ", "#### ", "#   #", "#   #", " ### "}},
        {'7', {"#####", "    #", "   # ", "  #  ", "  #  ", "  #  ", "  #  "}},
        {'8', {" ### ", "#   #", "#   #", " ### ", "#   #", "#   #", " ### "}},
        {'9', {" ### ", "#   #", "#   #", " ####", "    #", "#   #", " ### "}},
        {'-', {"     ", "     ", "     ", " ### ", "     ", "     ", "     "}},
        {'.', {"     ", "     ", "     ", "     ", "     ", "  ## ", "  ## "}},
        {' ', {"     ", "     ", "     ", "     ", "     ", "     ", "     "}}
    };#1#

    const auto it = font5x7.find(c);
    if (it == font5x7.end()) return;

    const auto &glyph = it->second;
    for (size_t dy = 0; dy < glyph.size(); ++dy) {
        for (size_t dx = 0; dx < glyph[dy].size(); ++dx) {
            if (glyph[dy][dx] == '#') {
                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        int px = x + dx * scale + sx;
                        int py = y + dy * scale + sy;
                        if (px >= 0 && py >= 0 && px < imgWidth) {
                            int idx = (py * imgWidth + px) * 3;
                            image[idx + 0] = 0;
                            image[idx + 1] = 0;
                            image[idx + 2] = 0;
                        }
                    }
                }
            }
        }
    }
}

void drawText(vector<unsigned char> &image,
              const int imgWidth,
              const int x,
              const int y,
              const string &text,
              const int scale) {
    for (size_t i = 0; i < text.size(); ++i) {
        drawChar(image, imgWidth, x + i * 6 * scale, y, text[i], scale);
    }
}

void writeHist(const map<long, long> &hist,
               const string &basePath,
               const string &reportPath,
               const string &algPath,
               const string &fileName,
               const string &suffix) {
    const string histPath = basePath + reportPath + algPath + "/histogram/";
    const string histFile = histPath + fileName + "_" + suffix + ".jpeg";
    createDir(histPath);

    const long nBins = 100;
    const long labelStep = 10;
    const long minBarWidth = 2;
    const long padding = 20;
    const long labelHeight = 20;

    const long minValue = hist.begin()->first;
    const long maxValue = hist.rbegin()->first;
    const long binSize = max(1L, (maxValue - minValue + 1) / nBins);

    vector<long> groupedBins(nBins, 0);
    long maxFreq = 0;

    for (const auto &[val, count]: hist) {
        long binIdx = min((val - minValue) / binSize, nBins - 1);
        groupedBins[binIdx] += count;
        maxFreq = max(maxFreq, groupedBins[binIdx]);
    }

    const long width = max(nBins * minBarWidth, 1000L);
    const long height = 400;
    const long barWidth = width / nBins;
    const long imageWidth = width + 2 * padding;
    const long imageHeight = height + 2 * padding + labelHeight;

    vector<unsigned char> image(imageWidth * imageHeight * 3, 255);

    for (long i = 0; i < nBins; ++i) {
        const long count = groupedBins[i];
        if (count == 0) continue;

        const long barHeight = static_cast<long>((static_cast<float>(count) / maxFreq) * height);
        const long xStart = padding + i * barWidth;
        const long xEnd = min(xStart + barWidth - 1, padding + width);

        for (long y = imageHeight - padding - labelHeight - 1; y >= imageHeight - padding - labelHeight - barHeight;
             --
             y) {
            for (long x = xStart; x <= xEnd; ++x) {
                long idx = (y * imageWidth + x) * 3;
                image[idx + 0] = 0; // R
                image[idx + 1] = 0; // G
                image[idx + 2] = 0; // B
            }
        }
    }

    // Draw axis lines
    for (long y = imageHeight - padding - labelHeight; y < imageHeight - padding - labelHeight + 2; ++y) {
        for (long x = padding; x < padding + width; ++x) {
            long idx = (y * imageWidth + x) * 3;
            image[idx + 0] = 0;
            image[idx + 1] = 0;
            image[idx + 2] = 0;
        }
    }

    // Draw labels
    for (long i = 0; i < nBins; i += labelStep) {
        long binStart = minValue + i * binSize;
        long binEnd = min(binStart + binSize - 1, maxValue);
        string label = to_string(binStart) + "-" + to_string(binEnd);
        int xPos = padding + i * barWidth + 2;
        drawText(image, imageWidth, xPos, imageHeight - labelHeight, label, 1);
    }

    stbi_write_jpg(histFile.c_str(), imageWidth, imageHeight, 3, image.data(), 90);
}

void writeBoxplot(const map<long, long> &hist,
                  const string &basePath,
                  const string &reportPath,
                  const string &algPath,
                  const string &fileName,
                  const string &suffix) {
    const string outPath = basePath + reportPath + algPath + "/boxplot/";
    const string outFile = outPath + fileName + "_" + suffix + ".jpeg";
    createDir(outPath);

    constexpr int width = 1000;
    constexpr int height = 300;
    constexpr int padding = 50;

    vector<unsigned char> image(width * height * 3, 255);

    // Make the raw data again
    vector<long> rawData;
    for (const auto &[val, count]: hist)
        rawData.insert(rawData.end(), count, val);

    sort(rawData.begin(), rawData.end());
    long n = rawData.size();
    if (n == 0) return;

    auto getQuantile = [&](float q) -> long {
        float pos = q * (n - 1);
        long idx = static_cast<long>(pos);
        float frac = pos - idx;
        if (idx + 1 < n)
            return static_cast<long>(rawData[idx] * (1 - frac) + rawData[idx + 1] * frac);
        return rawData[idx];
    };

    long Q1 = getQuantile(0.25);
    long Q2 = getQuantile(0.5);
    long Q3 = getQuantile(0.75);
    long IQR = Q3 - Q1;
    long minNonOut = *lower_bound(rawData.begin(), rawData.end(), Q1 - 1.5 * IQR);
    long maxNonOut = *upper_bound(rawData.begin(), rawData.end(), Q3 + 1.5 * IQR);

    vector<long> outliers;
    for (long v: rawData)
        if (v < minNonOut || v > maxNonOut)
            outliers.push_back(v);

    // Escala horizontal (de minNonOut a maxNonOut)
    auto scale = [&](long v) -> int {
        return padding + static_cast<int>((float) (v - minNonOut) / (maxNonOut - minNonOut) * (
                                              width - 2 * padding));
    };

    int boxY1 = height / 2 - 30;
    int boxY2 = height / 2 + 30;

    // Caixa do boxplot (Q1–Q3)
    int x1 = scale(Q1);
    int x2 = scale(Q3);
    for (int y = boxY1; y <= boxY2; ++y)
        for (int x = x1; x <= x2; ++x) {
            int idx = (y * width + x) * 3;
            image[idx + 0] = 200;
            image[idx + 1] = 200;
            image[idx + 2] = 200;
        }

    // vertical lines (whiskers e median)
    auto drawVLine = [&](int x, int y1, int y2, RGB color) {
        for (int y = y1; y <= y2; ++y) {
            int idx = (y * width + x) * 3;
            image[idx + 0] = color.r;
            image[idx + 1] = color.g;
            image[idx + 2] = color.b;
        }
    };
    drawVLine(scale(Q1), boxY1, boxY2, {0, 0, 0});
    drawVLine(scale(Q2), boxY1, boxY2, {255, 0, 0});
    drawVLine(scale(Q3), boxY1, boxY2, {0, 0, 0});

    // Linhas horizontais (whiskers)
    int yMid = (boxY1 + boxY2) / 2;
    for (int x = scale(minNonOut); x <= scale(Q1); ++x) {
        int idx = (yMid * width + x) * 3;
        image[idx + 0] = 0;
        image[idx + 1] = 0;
        image[idx + 2] = 0;
    }
    for (int x = scale(Q3); x <= scale(maxNonOut); ++x) {
        int idx = (yMid * width + x) * 3;
        image[idx + 0] = 0;
        image[idx + 1] = 0;
        image[idx + 2] = 0;
    }

    drawVLine(scale(minNonOut), yMid - 5, yMid + 5, {0, 0, 0});
    drawVLine(scale(maxNonOut), yMid - 5, yMid + 5, {0, 0, 0});

    // Outliers
    for (long v: outliers) {
        int x = scale(v);
        for (int dy = -2; dy <= 2; ++dy)
            for (int dx = -2; dx <= 2; ++dx) {
                int px = x + dx, py = yMid + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int idx = (py * width + px) * 3;
                    image[idx + 0] = 0;
                    image[idx + 1] = 0;
                    image[idx + 2] = 255;
                }
            }
    }

    // Labels
    drawText(image, width, scale(minNonOut) - 10, height - 10, to_string(minNonOut));
    drawText(image, width, scale(Q2) - 10, height - 10, "med: " + to_string(Q2));
    drawText(image, width, scale(maxNonOut) - 10, height - 10, to_string(maxNonOut));

    stbi_write_jpg(outFile.c_str(), width, height, 3, image.data(), 90);
}
*/
