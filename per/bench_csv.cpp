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

enum class NoteType {
    INTERNAL = 0,
    INPUT = 1,
    OUTPUT = 2,
};

struct Node {
    NoteType type;
    long asap;
    long alap;
    long slack;


    [[nodiscard]] string typeToCsv() const {
        string toString;
        switch (type) {
            case NoteType::INTERNAL:
                toString = "INTERNAL;";
            case NoteType::INPUT:
                toString = "INPUT;";
            case NoteType::OUTPUT:
                toString = "OUTPUT;";
            default:
                toString = "UNKNOWN;";
        }
        toString += to_string(asap) + ";";
        toString += to_string(alap) + ";";
        toString += to_string(slack) + "\n";
        return toString;
    }
};


void writeCsv(const string &basePath,
              const string &fileName,
              const ReportData &data) {
    string reportFullPath = basePath + reportPath + algPath + "/csv/";
    string reportFile = reportFullPath + fileName + ".csv";

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

void addNodeIfNotExists(
    unordered_map<long, Node> &nodesMap,
    const long nodeNum,
    const NoteType type,
    const Graph &g
) {
    if (nodesMap.find(nodeNum) == nodesMap.end()) {
        nodesMap[nodeNum] = Node{
            type,
            g.asap.at(nodeNum),
            g.alap.at(nodeNum),
            g.slack.at(nodeNum)
        };
    }
}

//Choose the parameters in fpgaPar.h definitions
int main() {
    const string rootPath = verifyPath(getProjectRoot());

    cout << rootPath << endl;

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        // Reading graphs
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));


        unordered_map<long, Node> nodesMap;

        for (auto const nodeNum: g.inputNodes) {
            addNodeIfNotExists(nodesMap, nodeNum, NoteType::INPUT, g);
        }

        for (auto const nodeNum: g.outputNodes) {
            addNodeIfNotExists(nodesMap, nodeNum, NoteType::OUTPUT, g);
        }

        for (auto const nodeNum: g.innerNodes) {
            addNodeIfNotExists(nodesMap, nodeNum, NoteType::INTERNAL, g);
        }


        std::vector<std::vector<std::pair<int, int> > > edgesSequences;

        int counter = 0;
        while (counter < 20) {
            vector<pair<long, long> > ed = g.getEdgesDepthFirst(false);
            counter++;
        }

        //ed = g.getEdgesDepthFirst(false);
        //string alg_type = "DEPTH_FIRST";


        //ed = g.getEdgesZigzag(convergence);

        //string fileName = g.dotName + "_" + to_string(i);
        //save edges csv files for the 10 better placements
        //writeCsv(rootPath, fileName, reports[i]);
    }

    return 0;
}
