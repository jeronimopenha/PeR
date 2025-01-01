#include  "util.h"
#include  "graph.h"
#include "yotoBase.h"

//YOTO + Depth first search
int main() {
    // std::string root_path = get_project_root(); // Supondo que essa função existe
    const std::string rootPath = verifyPath(getProjectRoot()); // Exemplo de função que busca o root path
    std::cout << rootPath << std::endl;
    const std::string benchPath = "benchmarks/fpga/eval/";
    const std::string benchExt = ".dot";

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // std::vector<std::vector<std::string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        std::cout << fst << std::endl;

        //Creating graph important variables
        dotPath = fst;
        dotName = snd.substr(0, snd.size() - 4);
        //reading graph variables
        graphClearData();
        getGraphDataInt();

        //execution parameters
        std::string outBaseFolder = "reports/fpga/yoto_base_zz/";
        int nExec = 1000;
        std::vector<ReportData> reports;
        for (int exec = 0; exec < nExec; exec++)
            reports.push_back(yotoBase(ZZ));

        //sort the reports by total cost because I want only the 10 better placements
        std::sort(reports.begin(), reports.end(), [](const ReportData &a, const ReportData &b) {
            return a.totalCost < b.totalCost;
        });


        for (int i = 0; i < 10; i++) {
            //savePlacedDot(reports[i].n2c, gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
            std::cout << dotName << std::endl;
            std::string reportName = dotName + "_" + std::to_string(i);
            //save reports for the 10 better placements
            writeJson(rootPath + outBaseFolder, reportName, reports[i]);
            //generate reports and files for vpr
            writeVprData(rootPath + outBaseFolder, reportName, reports[i]);
        }
    }
    return 0;
}
