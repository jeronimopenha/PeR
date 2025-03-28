#include  "util.h"
#include  "graph.h"
#include "yotoBase.h"
#include "yottBase.h"
#include "saBase.h"
#include <omp.h>

using namespace std;

//Choose the algorithm in util.h defines
int main() {
    // string root_path = get_project_root();
    const string rootPath = verifyPath(getProjectRoot());
    cout << rootPath << endl;
    const string benchPath = "benchmarks/fpga/eval/";
    //const string benchPath = "benchmarks/fpga/bench_test/";
    const string benchExt = ".dot";

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // vector<vector<string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        //Creating graph important variables
        Graph g = Graph(fst, snd.substr(0, snd.size() - 4));
        //reading graph variables
        g.getGraphDataInt();
        g.findLongestPath();

        //execution parameters
#ifdef YOTO_BASE_DF
        string outBaseFolder = "reports/fpga/yoto_base/";
#elifdef YOTO_BASE_DF_P
        string outBaseFolder = "reports/fpga/yoto_base_p/";
#elifdef YOTO_BASE_ZZ
        string outBaseFolder = "reports/fpga/yoto_base_zz/";
#elifdef YOTO_BASE_ZZ_CACHE
        string outBaseFolder = "reports/fpga/yoto_base_zz_cache/";
#elifdef YOTT_BASE
        string outBaseFolder = "reports/fpga_total_cost_metric/yott_base/";
        //string outBaseFolder = "reports/fpga/yott_base/";
#elifdef SA_BASE
        string outBaseFolder = "reports/fpga/sa_base/";
#endif

        int nExec = 100;
        vector<ReportData> reports;

        int nThreads = max(1, omp_get_num_procs() - 1);
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
            for (int exec = 0; exec < nExec; exec++) {
                ReportData report;
#if defined(YOTO_BASE_DF)||defined(YOTO_BASE_DF_P)||defined(YOTO_BASE_ZZ)||defined(YOTO_BASE_ZZ_CACHE)
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
            writeJson(rootPath + outBaseFolder, reportName, reports[i]);
            //generate reports and files for vpr
#ifndef YOTO_BASE_ZZ_CACHE
            writeVprData(rootPath + outBaseFolder, reportName, reports[i], g);
#endif
        }
    }
    return 0;
}
