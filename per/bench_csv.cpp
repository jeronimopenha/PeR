#include <definitions.h>
#include <util.h>
#include <fpgaPar.h>
#include <fpgaGraph.h>
#include  <fpgaUtil.h>
#include <fpgaYoto.h>
#include <omp.h>
//#include "fpga/fpgaYott.h"
//#include "fpga/fpgaSa.h"

using namespace std;

struct GraphStats {
    long nNodes = 0;
    long nEdges = 0;
    long nInput = 0;
    long nOutput = 0;
    long nIO = 0;

    double inDegreeAvg = 0.0;
    double outDegreeAvg = 0.0;

    double inDegreeAvgNonZero = 0.0;
    double outDegreeAvgNonZero = 0.0;

    double degreeAssortativityOutIn = 0.0;

    [[nodiscard]] string toString() const {
        string toString;

        toString += to_string(nNodes) + "; ";
        toString += to_string(nEdges) + "; ";
        toString += to_string(nInput) + "; ";
        toString += to_string(nOutput) + "; ";
        toString += to_string(nIO) + "; ";
        toString += to_string(inDegreeAvg) + "; ";
        toString += to_string(outDegreeAvg) + "; ";
        toString += to_string(inDegreeAvgNonZero) + "; ";
        toString += to_string(outDegreeAvgNonZero) + "; ";
        toString += to_string(degreeAssortativityOutIn) + "\n";

        return toString;
    }
};

enum class NodeType {
    INTERNAL = 0,
    INPUT = 1,
    OUTPUT = 2,
    UNKNOWN = 3
};

struct Node {
    long nodeNum;
    NodeType type;
    long asap;
    long alap;
    long slack;


    [[nodiscard]] string toString() const {
        string toString;
        switch (type) {
            case NodeType::INTERNAL:
                toString = "INTERNAL; ";
                break;
            case NodeType::INPUT:
                toString = "INPUT; ";
                break;
            case NodeType::OUTPUT:
                toString = "OUTPUT; ";
                break;
            default:
                toString = "UNKNOWN; ";
        }
        toString += to_string(asap) + "; ";
        toString += to_string(alap) + "; ";
        toString += to_string(slack) + "; ";
        toString += to_string(nodeNum);
        return toString;
    }
};

void addNodeIfNotExists(
    unordered_map<long, Node> &nodesMap,
    const long nodeNum,
    const NodeType type,
    const Graph &g
) {
    if (nodesMap.find(nodeNum) == nodesMap.end()) {
        nodesMap[nodeNum] = Node{
            nodeNum,
            type,
            g.asap.at(nodeNum),
            g.alap.at(nodeNum),
            g.slack.at(nodeNum)
        };
    }
}

double pearsonCorrelation(
    const std::vector<double> &x,
    const std::vector<double> &y
) {
    const std::size_t n = x.size();

    if (n == 0 || y.size() != n) {
        return 0.0;
    }

    double sumX = 0.0;
    double sumY = 0.0;
    double sumXX = 0.0;
    double sumYY = 0.0;
    double sumXY = 0.0;

    for (std::size_t i = 0; i < n; i++) {
        sumX += x[i];
        sumY += y[i];
        sumXX += x[i] * x[i];
        sumYY += y[i] * y[i];
        sumXY += x[i] * y[i];
    }

    double numerator = n * sumXY - sumX * sumY;

    double denominatorPartX = n * sumXX - sumX * sumX;
    double denominatorPartY = n * sumYY - sumY * sumY;

    double denominator = std::sqrt(denominatorPartX * denominatorPartY);

    if (denominator == 0.0) {
        return 0.0;
    }

    return numerator / denominator;
}

double degreeAssortativityOutIn(
    const std::vector<std::pair<long, long> > &edges,
    const std::vector<long> &inDegree,
    const std::vector<long> &outDegree
) {
    std::vector<double> x;
    std::vector<double> y;

    x.reserve(edges.size());
    y.reserve(edges.size());

    for (const auto &[src, dst]: edges) {
        x.push_back(static_cast<double>(outDegree[src]));
        y.push_back(static_cast<double>(inDegree[dst]));
    }

    return pearsonCorrelation(x, y);
}

GraphStats computeGraphStats(
    const FPGAGraph &g
) {
    const long nNodes = g.nNodes;
    const long nInput = static_cast<long>(g.inputNodes.size());
    const long nOutput = static_cast<long>(g.outputNodes.size());
    const long nIO = nInput + nOutput;
    const std::vector<std::pair<long, long> > &edges = g.gEdges;
    GraphStats stats;

    stats.nNodes = nNodes;
    stats.nEdges = static_cast<long>(edges.size());
    stats.nInput = nInput;
    stats.nOutput = nOutput;
    stats.nIO = nIO;

    std::vector<long> inDegree(nNodes, 0);
    std::vector<long> outDegree(nNodes, 0);

    for (const auto &[src, dst]: edges) {
        outDegree[src]++;
        inDegree[dst]++;
    }

    long totalIn = 0;
    long totalOut = 0;
    long nodesWithIn = 0;
    long nodesWithOut = 0;

    for (long i = 0; i < nNodes; i++) {
        totalIn += inDegree[i];
        totalOut += outDegree[i];

        if (inDegree[i] > 0) {
            nodesWithIn++;
        }

        if (outDegree[i] > 0) {
            nodesWithOut++;
        }
    }

    if (nNodes > 0) {
        stats.inDegreeAvg = static_cast<double>(totalIn) / nNodes;
        stats.outDegreeAvg = static_cast<double>(totalOut) / nNodes;
    }

    stats.inDegreeAvgNonZero = nodesWithIn > 0
                                   ? static_cast<double>(totalIn) / nodesWithIn
                                   : 0.0;

    stats.outDegreeAvgNonZero = nodesWithOut > 0
                                    ? static_cast<double>(totalOut) / nodesWithOut
                                    : 0.0;

    stats.degreeAssortativityOutIn = degreeAssortativityOutIn(
        edges,
        inDegree,
        outDegree
    );

    return stats;
}

//Choose the parameters in fpgaPar.h definitions
int main() {
    const string rootPath = verifyPath(getProjectRoot());

    cout << rootPath << endl;

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);

    string reportPath = "/home/jeronimo/GIT/PER_GPU/DF_csv_benchmarks/";
    createDir(reportPath);

    string stattsFile = reportPath + "BENCHMARKS_STATS.csv";
    ofstream statsWriter(stattsFile);
    statsWriter << "Benchmark; nNodes; nEdges; nInput; nOutput; nIO; inDegreeAvg; outDegreeAvg; inDegreeAvgNonZero; outDegreeAvgNonZero; degreeAssortativityOutIn" << endl;

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        // Reading graphs
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));

        unordered_map<long, Node> nodesMap;

        for (auto const nodeNum: g.inputNodes) {
            addNodeIfNotExists(nodesMap, nodeNum, NodeType::INPUT, g);
        }

        for (auto const nodeNum: g.outputNodes) {
            addNodeIfNotExists(nodesMap, nodeNum, NodeType::OUTPUT, g);
        }

        for (auto const nodeNum: g.innerNodes) {
            addNodeIfNotExists(nodesMap, nodeNum, NodeType::INTERNAL, g);
        }


        GraphStats stats = computeGraphStats(g);
        statsWriter << g.dotName << "; " << stats.toString() << endl;

        string reportFullPath = reportPath + g.dotName + "/";
        createDir(reportFullPath);

        string edgesFile = reportPath + g.dotName + "_EDGES.csv";

        //full edges
        ofstream edgesAlgWriter(edgesFile);
        if (edgesAlgWriter.is_open()) {
            edgesAlgWriter << "TYPE; ASAP; ALAP; SLACK; A; TYPE; ASAP; ALAP; SLACK; B" << endl;
            for (auto [fst, snd]: g.gEdges) {
                if (fst == -1) {
                    edgesAlgWriter << Node{
                        -1,
                        NodeType::UNKNOWN,
                        -1,
                        -1,
                        -1
                    }.toString() << "; " << nodesMap[snd].toString() << endl;
                } else {
                    edgesAlgWriter << nodesMap[fst].toString() << "; " << nodesMap[snd].toString() << endl;
                }
            }
            edgesAlgWriter.close();
        } else {
            cerr << "Error opening file for writing: " << edgesFile << endl;
        }

        //statistics

        //for DF algorithm
        std::vector<std::vector<std::pair<long, long> > > edgesSequences;

        int counter = 0;
        int tries = 0;

        while (counter < 20 && tries < 100) {
            vector<pair<long, long> > ed = g.getEdgesDepthFirst(false);
            if (std::find(edgesSequences.begin(), edgesSequences.end(), ed) == edgesSequences.end()) {
                edgesSequences.push_back(ed);
                counter++;
                tries = 0;
            } else {
                tries++;
            }
        }

        for (size_t i = 0; i < edgesSequences.size(); i++) {
            auto const &csv = edgesSequences[i];
            string reportFile = reportFullPath + g.dotName + "_" + to_string(i) + ".csv";

            ofstream csvAlgWriter(reportFile);
            if (csvAlgWriter.is_open()) {
                csvAlgWriter << "TYPE; ASAP; ALAP; SLACK; A; TYPE; ASAP; ALAP; SLACK; B" << endl;
                for (auto [fst, snd]: csv) {
                    if (fst == -1) {
                        csvAlgWriter << Node{
                            -1,
                            NodeType::UNKNOWN,
                            -1,
                            -1,
                            -1
                        }.toString() << "; " << nodesMap[snd].toString() << endl;
                    } else {
                        csvAlgWriter << nodesMap[fst].toString() << "; " << nodesMap[snd].toString() << endl;
                    }
                }
                csvAlgWriter.close();
            } else {
                cerr << "Error opening file for writing: " << reportFile << ".json" << endl;
            }
        }
    }
    statsWriter.close();

    return 0;
}
