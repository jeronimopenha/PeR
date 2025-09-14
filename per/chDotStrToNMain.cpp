#include  <common/util.h>
#include  <common/graph.h>

using namespace std;

int main() {
    const std::string rootPath = verifyPath(getProjectRoot());
    std::cout << rootPath << std::endl;
    const std::string benchPath = "benchmarks/fpga/bench_test/";
    const std::string benchExt = ".dot";

    auto files = getFilesListByExtension(rootPath + benchPath, benchExt);
    // std::vector<std::vector<std::string>> files = {{"path/to/file.dot", "file.dot"}};

    for (const auto &[fst, snd]: files) {
        std::cout << fst << std::endl;

        auto g = Graph(fst, snd.substr(0, snd.size() - 4), true);

        g.saveToDot(g.dotPath);
    }
    return 0;
}
