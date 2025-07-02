#include <algorithm>

#include <include/fpga/parametersFpga.h>
#include  <common/util.h>
#include  <qca/qcaGraph.h>
#include <qca/qcaYoto.h>
#include <qca/qcaYott.h>
#include <qca/qcaSa.h>

#include <omp.h>

using namespace std;

//Choose the algorithm in util.h defines
int main()
{
    const string rootPath = verifyPath(getProjectRoot());
    const string benchExt = ".dot";

#ifdef DEBUG
    const string benchPath = "benchmarks/qca/bench_test/";
#else
    const string benchPath = "benchmarks/qca/eval/";
#endif
    const string reportPath = "reports/qca";
    string algPath;

    cout << rootPath << endl;

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);

    for (const auto& [fst, snd] : files)
    {
        cout << fst << endl;

        //Creating graph important variables
        auto g = QCAGraph(fst, snd.substr(0, snd.size() - 4));

        int nExec;
        //execution parameters
#ifdef QCA_YOTO_ZZ
        algPath = "/yoto_zz";
#elifdef QCA_SA
        algPath = "/sa";
#elifdef QCA_YOTT
        algPath = "/yott";
#endif

#ifdef DEBUG
        nExec = 1;
#elifdef  QCA_SA
        nExec = 100;
#else
        nExec = 1000;
#endif
        int nExtraLayers = 0;

        vector<vector<QcaReportData>> reports(MAX_EXTRA_LAYERS, vector<QcaReportData>(nExec));

        while (nExtraLayers < MAX_EXTRA_LAYERS)
        {
#ifndef DEBUG
            int nThreads = max(1, omp_get_num_procs());
            omp_set_num_threads(nThreads);

#pragma omp parallel
            {
#pragma omp for schedule(dynamic)
#endif
            for (int exec = 0; exec < nExec; exec++)
            {
                QcaReportData report;

#if defined(QCA_YOTO_ZZ)
                report = qcaYoto(g);
#elif  defined(QCA_SA)
                report = qcaSa(g);
#elif  defined(QCA_YOTT)
                report = qcaYott(g);
#endif

#ifndef DEBUG
#pragma omp critical
#endif
                {
                    reports[nExtraLayers][exec] = report;
                }
            }
#ifndef DEBUG
            }
#endif
            nExtraLayers++;
            g.insertDummyLayerAtLevel(randomInt(0, g.minOutputLevel));
        }

#ifndef DEBUG
        for (auto& rep : reports)
        {
            sort(rep.begin(), rep.end(), [](const QcaReportData& a, const QcaReportData& b)
            {
                /*if (a.success != b.success)
                    return a.success > b.success; // true vem antes de false*/
                return a.wrongEdges < b.wrongEdges; // menos arestas erradas vem primeiro
            });
        }

        for (int extraLayer = 0; extraLayer < MAX_EXTRA_LAYERS; extraLayer++)
        {
            cout << g.dotName << endl;
            string fileName = g.dotName + "_" + to_string(extraLayer);
            qcaWriteJson(rootPath, reportPath, algPath, fileName, extraLayer, reports[extraLayer][0]);

            auto n2c = reports[extraLayer][0].n2c;
            auto ed = reports[extraLayer][0].edges;
            auto nCellsSqrt = reports[extraLayer][0].nCellsSqrt;
            string finalPath = rootPath + reportPath + algPath + "/dot/";
            string dotFile = finalPath + fileName + ".dot";
            createDir(finalPath);

            qcaExportUSEToDot(dotFile, n2c, ed, nCellsSqrt);
        }
#endif
    }
    return 0;
}
