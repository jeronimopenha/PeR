#include  "util.h"
#include  "graph.h"
#include "yotoBase.h"


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
        getGraphDataInt();

        //execution parameters
        int nExec = 100;
        std::string baseFolder = "reports/fpga/yoto_base";

        yotoBase();

        /*
        FPGAPPeR fpgaPer(g);

        std::vector<std::string> placers = {"yoto"};

        for (const auto &placer: placers) {
            std::unordered_map<int, ReportData> reports;
            if (placer == "yoto") {
                reports = fpgaPer.perYoto(nExec);
                std::string fileNamePrefix = "yoto_Depth_First";
                save_reports(fpgaPer.graph.dotName, verifyPath(rootPath) + baseFolder, fileNamePrefix, reports);
            }*/
    }

    /*FPGAPeR per(g);

    // Componente desconectado para DAG
    auto disconnected_components = g.getWeaklyConnectedComponents();

    int n_exec = 11;
    std::string base_folder = "reports/fpga/outputs/";
    std::vector<std::string> placers = {"yoto"};
    std::vector<EdAlgEnum> yoto_algs = {EdAlgEnum::DEPTH_FIRST_WITH_PRIORITY};

    for (const auto &placer: placers) {
        std::unordered_map<std::string, ReportType> reports;
        // Supondo que ReportType é um tipo que armazena o relatório
        std::string file_name_prefix;
        auto start = std::chrono::high_resolution_clock::now();

        if (placer == "yoto") {
            for (const auto &alg: yoto_algs) {
                reports = per.per(PeR_Enum::YOTO, {alg}, n_exec);
                auto end = std::chrono::high_resolution_clock::now();
                std::cout << "Tempo: "
                        << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
                        << " segundos" << std::endl;

                file_name_prefix = "yoto_" + std::to_string(static_cast<int>(alg));
                save_reports(per, verifyPath(root_path) + base_folder, file_name_prefix, reports);
            }
        } else if (placer == "yott") {
            reports = per.per(PeR_Enum::YOTT, {}, n_exec);
            file_name_prefix = "yott";
            save_reports(per, verifyPath(root_path) + base_folder, file_name_prefix, reports);
        } else if (placer == "sa") {
            reports = per.per(PeR_Enum::SA, {}, n_exec);
            file_name_prefix = "sa";
            save_reports(per, verifyPath(root_path) + base_folder, file_name_prefix, reports);
        }
    }
}

//generate_pic(); // Supondo que essa função exista em C++
*/
    return 0;
}
