#include <common/parameters.h>
#include  <common/util.h>
#include  <qca/qcaGraph.h>
#include <qca/qcaYoto.h>
#include <qca/qcaSa.h>

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
        auto g = QCAGraph(fst, snd.substr(0, snd.size() - 4));
        //reading graph variables

        int nExec;
        //execution parameters
#ifdef QCA_YOTO_ZZ
        algPath = "/yoto_df";
#elifdef QCA_SA
        algPath = "/sa";
#endif

#ifdef DEBUG
        nExec = 100;
#elifdef  QCA_SA
            nExec = 100;
#else
        nExec = 1000;
#endif
        int nExtraLayers = 0;
        bool found = false;
        vector<QcaReportData> reports;

        while (nExtraLayers < MAX_EXTRA_LAYERS && !found) {
            reports.clear();
#ifndef DEBUG
            int nThreads = max(1, omp_get_num_procs() - 1);
            omp_set_num_threads(nThreads);

#pragma omp parallel
            {
#pragma omp for schedule(dynamic)
#endif
            for (int exec = 0; exec < nExec; exec++) {
                QcaReportData report;
#if defined(QCA_YOTO_ZZ)
                report = qcaYoto(g);
#elif  defined(QCA_SA)
                report = qcaSa(g);
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
            reports.erase(
                remove_if(reports.begin(), reports.end(), [](const QcaReportData &r) {
                    return !r.success;
                }),
                reports.end()
            );
            if (!reports.empty())
                found = true;
            else {
                g.insertDummyLayerAtLevel(randomInt(1,g.minOutputLevel));
                nExtraLayers++;
            }
        }
        //sort the reports by total cost because I want only the 10 better placements
        // sort(reports.begin(), reports.end(), [](const QcaReportData &a, const QcaReportData &b) {
        //     return a.totalCost < b.totalCost;
        // });
        int limit = (10 < reports.size()) ? 10 : reports.size();
        for (int i = 0; i < limit; i++) {
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            cout << g.dotName << endl;
            string fileName = g.dotName + "_" + to_string(i);
        }
    }
    return 0;
}
