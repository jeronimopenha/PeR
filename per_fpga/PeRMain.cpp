#include "parameters.h"
#include  "util.h"
#include  "graph.h"

#if defined(YOTO_DF) || defined(YOTO_DF_PRIO) || defined(YOTO_ZZ)
#include "yoto.h"
#endif

#if defined(YOTT)
#include "yott.h"
#endif

#if defined(SA)
#include "sa.h"
#endif

#include <omp.h>

using namespace std;

//Choose the algorithm in util.h defines
int main() {
    // string root_path = get_project_root();
    const string rootPath = verifyPath(getProjectRoot());
    const string benchExt = ".dot";

    const string benchPath = "benchmarks/fpga/eval/";
    //const string benchPath = "benchmarks/fpga/bench_test/";

    const string reportPath = "reports/fpga";
    string algPath;


    cout << rootPath << endl;


    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // vector<vector<string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        //Creating graph important variables
        Graph g = Graph(fst, snd.substr(0, snd.size() - 4));
        //reading graph variables
        g.getGraphDataInt();
        g.findLongestPath();

        int nExec;
        //execution parameters
#ifdef YOTO_DF
        algPath = "/yoto_df";
#elifdef YOTO_DF_PRIO
        algFolder =  "/yoto_df_prio";
#elifdef YOTO_ZZ
        algFolder  = "/yoto_zz";
#elifdef YOTT
        algFolder  = "/yott";
#elifdef SA
        algFolder = "/sa";
#endif

#ifdef CACHE
            algFolder += "_cache";
#endif


#ifdef SA
            nExec = 100;
#else
        nExec = 1000;
#endif

        vector<ReportData> reports;

        int nThreads = max(1, omp_get_num_procs() - 1);
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
            for (int exec = 0; exec < nExec; exec++) {
                ReportData report;
#if defined(YOTO_DF)||defined(YOTO_BASE_DF_P)||defined(YOTO_ZZ)||defined(CACHE)
                report = yotoBase(g);
#elifdef YOTT_BASE
                report = yottBase(g);
#elifdef SA_BASE
                report = saBase(g);
#endif
#pragma omp critical
                {
                    reports.push_back(report);
                }
            }
        }

        //sort the reports by total cost because I want only the 10 better placements
        sort(reports.begin(), reports.end(), [](const ReportData &a, const ReportData &b) {
            return a.totalCost < b.totalCost;
        });

        //todo tries and swaps SA!!!
        for (int i = 0; i < 10; i++) {
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            cout << g.dotName << endl;
            string reportName = g.dotName + "_" + to_string(i);
            //save reports for the 10 better placements
            //TODO !!!!!!! Consertar os saves e implementar a criação automática de pastas
            writeJson(rootPath, reportPath, algPath, reportName, reports[i]);
            //generate reports and files for vpr
#ifndef CACHE
            //TODO Revisar todo o código e rodar tudo novamente agora mais organizado
            writeVprData(rootPath + reportPath, reportName, reports[i], g);
#endif
        }
    }
    return 0;
}
