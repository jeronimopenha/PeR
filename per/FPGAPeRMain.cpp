#include <common/parameters.h>
#include  <fpga/fpgaUtil.h>
#include  <fpga/fpgaGraph.h>
#include "fpga/fpgaYoto.h"
#include "fpga/fpgaYott.h"
#include "fpga/fpgaSa.h"


#include <omp.h>

using namespace std;

//Choose the algorithm in util.h defines
int main() {
    const string rootPath = verifyPath(getProjectRoot());
    const string benchExt = ".dot";

#ifdef DEBUG
    const string benchPath = "benchmarks/fpga/bench_test/";
#else
#ifdef TRETS
    const string benchPath = "benchmarks/fpga/eval/TRETS/";
#elifdef EPFL
    const string benchPath = "benchmarks/fpga/eval/EPFL/";
#endif


#endif
    const string reportPath = "reports/fpga";
    string algPath;


    cout << rootPath << endl;


    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // vector<vector<string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        //Creating graph important variables
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));


        //execution parameters
#ifdef TRETS
            algPath = "/TRETS";
#elifdef EPFL
        algPath = "/EPFL";
#endif

#ifdef FPGA_YOTO_DF
        algPath += "/yoto_df";
#elifdef FPGA_YOTO_DF_PRIO
        algPath += "/yoto_df_prio";
#elifdef FPGA_YOTO_ZZ
        algPath  += "/yoto_zz";
#elifdef FPGA_YOTT
        algPath += "/yott";
#elifdef FPGA_YOTT_IO
        algPath += "/yott_io";
#elifdef FPGA_SA
        algPath += "/sa";
#endif


#ifdef CACHE
        algPath += "_cache_" + to_string(CACHE_LINES_EXP) + "x" + to_string(CACHE_COLUMNS_EXP);
#endif


#ifdef DEBUG
        constexpr int nExec = 1;
#elifdef  FPGA_SA
        constexpr int    nExec = 10;
#else
        constexpr int nExec = 1000;
#endif

        vector<FpgaReportData> reports;
        auto comp = [](const FpgaReportData &a, const FpgaReportData &b) {
            return a.totalCost < b.totalCost;
        };
#ifndef DEBUG

        int nThreads = max(1, omp_get_num_procs());
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
#endif


        for (int exec = 0; exec < nExec; exec++) {
            //cout << exec << " ";
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
#ifndef DEBUG
                if (reports.size() < 10 || report.totalCost < reports.back().totalCost) {
                    // encontra a posição onde deve ser inserido
                    auto pos = std::lower_bound(reports.begin(), reports.end(), report, comp);
                    reports.insert(pos, report);

                    // se passou de 10, remove o pior
                    if (reports.size() > 10)
                        reports.pop_back();
                }
#endif
                //reports.push_back(report);
            }
        }
#ifndef DEBUG
        }
#endif

/#ifndef DEBUG
        const int limit = min(10, static_cast<int>(reports.size()));
        for (int i = 0; i < limit; i++) {
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            cout << g.dotName << endl;
            string fileName = g.dotName + "_" + to_string(i);

            //save reports for the 10 better placements
            fpgaWriteJson(rootPath, reportPath, algPath, fileName, reports[i]);

#if !defined(CACHE)
            //generate reports and files for vpr
            fpgaWriteVprData(rootPath, reportPath, algPath, fileName, reports[i], g);
#endif
        }
#endif
    }
    return 0;
}
