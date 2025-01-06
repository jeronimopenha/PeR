#include  "util.h"
#include  "graph.h"
#include "yotoBase.h"

//Choose the algorithm in util.h defines
int main() {
    // std::string root_path = get_project_root();
    const std::string rootPath = verifyPath(getProjectRoot());
    std::cout << rootPath << std::endl;
    const std::string benchPath = "benchmarks/fpga/eval/";
    const std::string benchExt = ".dot";

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // std::vector<std::vector<std::string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        std::cout << fst << std::endl;

        //Creating graph important variables
        Graph g = Graph(fst, snd.substr(0, snd.size() - 4));
        //reading graph variables
        g.getGraphDataInt();

        //execution parameters
#ifdef YOTO_BASE_DF
        std::string outBaseFolder = "reports/fpga/yoto_base/";
#elifdef YOTO_BASE_DF_P
        std::string outBaseFolder = "reports/fpga/yoto_base_p/";
#elifdef YOTO_BASE_ZZ
        std::string outBaseFolder = "reports/fpga/yoto_base_zz/";
#elifdef YOTO_BASE_ZZ_CACHE
        std::string outBaseFolder = "reports/fpga/yoto_base_zz_cache/";
#elifdef YOTT_BASE
        std::string outBaseFolder = "reports/fpga/yott_base/";
#endif

        int nExec = 1000;
        std::vector<ReportData> reports;
        for (int exec = 0; exec < nExec; exec++)
            reports.push_back(yotoBase(g));

        //sort the reports by total cost because I want only the 10 better placements
        std::sort(reports.begin(), reports.end(), [](const ReportData &a, const ReportData &b) {
            return a.totalCost < b.totalCost;
        });


        for (int i = 0; i < 10; i++) {
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            std::cout << g.dotName << std::endl;
            std::string reportName = g.dotName + "_" + std::to_string(i);
            //save reports for the 10 better placements
            writeJson(rootPath + outBaseFolder, reportName, reports[i]);
            //generate reports and files for vpr
#ifndef YOTO_BASE_ZZ_CACHE
            //writeVprData(rootPath + outBaseFolder, reportName, reports[i]);
#endif
        }
    }
    return 0;
}
