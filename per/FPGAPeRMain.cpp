#include <fpga/fpgaPar.h>
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

    cout << rootPath << endl;

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        //Creating graph important variables
        auto g = FPGAGraph(fst, snd.substr(0, snd.size() - 4));


        //execution parameters
#ifdef DEBUG
        constexpr int nExec = 1;
#elifdef RUN_10
        constexpr int nExec = 10;
#elifdef RUN_100
        constexpr int nExec = 100;
#elifdef RUN_1000
        constexpr int nExec = 1000;
#endif

        vector<FpgaReportData> reports;

#ifdef REPORT
        auto comp = [](const FpgaReportData &a, const FpgaReportData &b) {
#ifdef FPGA_TOTAL_COST
                return a.totalCost < b.totalCost;
#elifdef FPGA_LONG_PATH_COST
            return a.lPCost < b.lPCost;
#endif
        };
#endif

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
#ifdef REPORT

#ifdef FPGA_TOTAL_COST
                    if (reports.size() < 10 || report.totalCost < reports.back().totalCost) {
#elifdef FPGA_LONG_PATH_COST
                if (reports.size() < 10 || report.lPCost < reports.back().lPCost) {
#endif
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

#ifdef REPORT
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
            fpgaWriteJson(rootPath, reportPath, algPath, fileName, reports[i]);

#if !defined(CACHE)
            //generate reports and files for vpr
            fpgaWriteVprData(rootPath, reportPath, algPath, fileName, reports[i], g);
#endif
#ifdef MAKE_METRICS
            writeHeatmap(reports[i].heatEnd, reports[i].c2n, g.nCellsSqrt, rootPath, reportPath, algPath, fileName,
                         "end");
            writeHeatmap(reports[i].heatBegin, reports[i].c2n, g.nCellsSqrt, rootPath, reportPath, algPath, fileName,
                         "begin");
            //writeHist(reports[i].hist[9], rootPath, reportPath, algPath, fileName, "9");
            //writeBoxplot(reports[i].hist[9], rootPath, reportPath, algPath, fileName, "9");
#endif
        }
#endif
    }
    return 0;
}
