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

#ifdef DEBUG
    const string benchPath = "benchmarks/fpga/bench_test/";
#else
        const string benchPath = "benchmarks/fpga/eval/";
#endif
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
        algPath =  "/yoto_df_prio";
#elifdef YOTO_ZZ
        algPath  = "/yoto_zz";
#elifdef YOTT
        algPath = "/yott";
#elifdef SA
        algPath = "/sa";
#endif

#ifdef CACHE
            algPath += "_cache";
#endif

#ifdef PLACE_IO_FIRST
        algPath += "_io_placed";
#endif

#ifdef DEBUG
        nExec = 1;
#elifdef  SA
            nExec = 100;
#else
        nExec = 1000;
#endif


        vector<ReportData> reports;

#ifndef DEBUG
        int nThreads = max(1, omp_get_num_procs() - 1);
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
#endif
        for (int exec = 0; exec < nExec; exec++) {
            ReportData report;
#if defined(YOTO_DF)||defined(YOTO_DF_PRIO)||defined(YOTO_ZZ)
            report = yotoBase(g);
#elifdef YOTT
            report = yottBase(g);
#elifdef SA
                report = saBase(g);
#endif
#ifndef DEBUG
#pragma omp critical
#endif
            {
                reports.push_back(report);
            }
        }
#ifndef DEBUG
    }
#endif

        //sort the reports by total cost because I want only the 10 better placements
        sort(reports.begin(), reports.end(), [](const ReportData &a, const ReportData &b) {
            return a.totalCost < b.totalCost;
        });
        int limit = (10 < reports.size()) ? 10 : reports.size();
        for (int i = 0; i < limit; i++) {
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            cout << g.dotName << endl;
            string fileName = g.dotName + "_" + to_string(i);
#ifndef DEBUG
            //save reports for the 10 better placements
            writeJson(rootPath, reportPath, algPath, fileName, reports[i]);
#endif
#if !defined(CACHE) && !defined (DEBUG)
            //generate reports and files for vpr
            writeVprData(rootPath, reportPath, algPath, fileName, reports[i], g);
#endif
        }
    }
    return
            0;
}
