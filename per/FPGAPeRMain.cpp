#include <common/parameters.h>
#include  <fpga/fpgaUtil.h>
#include  <fpga/fpgaGraph.h>
#include "fpga/fpgaYoto.h"
#include "fpga/fpgaYott.h"
#include "fpga/fpgaSa.h"


#include <omp.h>

using namespace std;

//Choose the algorithm in util.h defines
int main()
{
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

    for (const auto& [fst, snd] : files)
    {
        cout << fst << endl;

        //Creating graph important variables
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));

        int nExec;
        //execution parameters
#ifdef FPGA_YOTO_DF
        algPath = "/yoto_df";
#elifdef FPGA_YOTO_DF_PRIO
        algPath =  "/yoto_df_prio";
#elifdef FPGA_YOTO_ZZ
        algPath  = "/yoto_zz";
#elifdef FPGA_YOTT
        algPath = "/yott";
#elifdef FPGA_YOTT_IO
        algPath = "/yott_io";
#elifdef FPGA_SA
        algPath = "/sa";
#endif

#ifdef CACHE
        algPath += "_cache_" + to_string(CACHE_LINES_EXP) + "x" + to_string(CACHE_COLUMNS_EXP);
#endif


#ifdef DEBUG
        nExec = 1;
#elifdef  FPGA_SA
            nExec = 100;
#else
        nExec = 1000;
#endif


        vector<FpgaReportData> reports;

#ifndef DEBUG
        int nThreads = max(1, omp_get_num_procs() - 1);
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
#endif
        for (int exec = 0; exec < nExec; exec++)
        {
            FpgaReportData report;
#if defined(FPGA_YOTO_DF)||defined(FPGA_YOTO_DF_PRIO)||defined(FPGA_YOTO_ZZ)
            report = fpgaYoto(g);
#elif  defined(FPGA_YOTT) || defined(FPGA_YOTT_IO)
            report = fpgaYott(g);
#elifdef FPGA_SA
                report = fpgaSa(g);
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
        sort(reports.begin(), reports.end(), [](const FpgaReportData& a, const FpgaReportData& b)
        {
            return a.totalCost < b.totalCost;
        });
        int limit = (10 < reports.size()) ? 10 : reports.size();
        for (int i = 0; i < limit; i++)
        {
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
    return 0;
}
