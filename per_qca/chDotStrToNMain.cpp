#include  "util.h"
#include  "graph.h"


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

        graphClearData();

        //Creating graph important variables
        dotPath = fst;
        dotName = snd.substr(0, snd.size() - 4);
        //reading graph variables
        getGraphDataStr();
        saveToDot(gEdges, dotPath);
    }
    return 0;
}
