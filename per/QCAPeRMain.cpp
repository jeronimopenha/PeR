#include <common/parameters.h>
#include  <common/util.h>
#include  <qca/qcaGraph.h>
#include <qca/qcaYoto.h>

#include <omp.h>

using namespace std;

//Choose the algorithm in util.h defines
int main() {
    // string root_path = get_project_root();
    const string rootPath = verifyPath(getProjectRoot());
    const string benchExt = ".dot";

#ifdef DEBUG
    const string benchPath = "benchmarks/qca/bench_test/";
#else
        const string benchPath = "benchmarks/qca/eval_dot/";
#endif
    const string reportPath = "reports/fqca";
    string algPath;

    cout << rootPath << endl;


    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // vector<vector<string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        cout << fst << endl;

        //Creating graph important variables
        QCAGraph g = QCAGraph(fst, snd.substr(0, snd.size() - 4));
        //reading graph variables

        int nExec;
        //execution parameters
#ifdef QCA_YOTO_DF
        algPath = "/yoto_df";
#endif

#ifdef DEBUG
        nExec = 1;
#elifdef  FPGA_SA
            nExec = 100;
#else
        nExec = 1000;
#endif

        vector<QcaReportData> reports;

#ifndef DEBUG
        int nThreads = max(1, omp_get_num_procs() - 1);
        omp_set_num_threads(nThreads);

#pragma omp parallel
        {
#pragma omp for schedule(dynamic)
#endif
        for (int exec = 0; exec < nExec; exec++) {
            QcaReportData report;
#if defined(QCA_YOTO_DF)
            //report = qcaYoto(g);
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
        sort(reports.begin(), reports.end(), [](const QcaReportData &a, const QcaReportData &b) {
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
#if !defined (DEBUG)
            //generate reports and files for vpr
            writeVprData(rootPath, reportPath, algPath, fileName, reports[i], g);
#endif
        }
    }
    return 0;
}
