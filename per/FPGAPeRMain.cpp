#include <common/definitions.h>
#include <common/util.h>
#include <fpga/fpgaPar.h>
#include  <fpga/fpgaGraph.h>
#include  <fpga/fpgaUtil.h>
#include "fpga/fpgaYoto.h"
//#include "fpga/fpgaYott.h"
//#include "fpga/fpgaSa.h"

using namespace std;

//Choose the parameters in fpgaPar.h definitions
int main() {
    const string rootPath = verifyPath(getProjectRoot());

    cout << rootPath << endl;

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        // Reading graphs
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));

        // reports vector
        vector<FpgaReportData> reports;


        //fixme The costs functions are not working well

        auto comp = [](const FpgaReportData &a, const FpgaReportData &b) {
            return a.totalCost < b.totalCost;
        };


#ifndef DEBUG
        //openmp Parallelization for release execution
        int nThreads = max(1, omp_get_num_procs());
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
#endif

        for (int exec = 0; exec < nExec; exec++) {
            FpgaReportData report;

            //defining which algorithm will be run
#if defined(FPGA_YOTO_DF) || defined(FPGA_YOTO_DF_PRIO) || defined(FPGA_YOTO_ZZ)
            report = fpgaYoto(g);W
#elif  defined(FPGA_YOTT) || defined(FPGA_YOTT_IO)
            report = fpgaYott(g);
#elifdef FPGA_SA
            report = fpgaSa(g);
#endif

#ifndef DEBUG
#pragma omp critical
#endif
            {
                if (reports.size() < 10 || report.totalCost < reports.back().totalCost) {
                    // look for the right insertion position
                    auto pos = std::lower_bound(reports.begin(), reports.end(), report, comp);
                    reports.insert(pos, report);

                    // if size is greather than 10, then remove the worst one. The last onde
                    if (reports.size() > 10)
                        reports.pop_back();
                }
            }
        }
#ifndef DEBUG
        }
#endif

#ifdef BEST_ONLY
        for (int i = 0; i < 1; i++) {
#else
            const int limit = min(10, static_cast<int>(reports.size()));
            for (int i = 0; i < limit; i++) {
#endif
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            cout << g.dotName << endl;
            string fileName = g.dotName + "_" + to_string(i);

            //save reports for the 10 better placements
            fpgaWriteReports(rootPath, fileName, reports[i]);

#if !defined(USE_CACHE)
            //generate reports and files for vpr
#ifdef VPR_V5
            fpgaWriteVpr5Data(rootPath, fileName, reports[i], g);
#elifdef VPR_V9
            fpgaWriteVpr9Data(rootPath, fileName, reports[i], g);
#endif

            /*
        * std::string folderPath = "/home/jeronimo/GIT/PeR/reports/fpga/EPFL/yoto_df_x1_debug/metrics/";
        std::string command = "python3 script.py \"" + folderPath + "\"";
        int result = std::system(command.c_str());
        return result;
             */
#endif
        }
    }
    return 0;
}
