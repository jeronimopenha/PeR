//
// Created by jeronimo on 01/11/24.
//
#include "util.h"

std::string getProjectRoot() {
    std::filesystem::path path = std::filesystem::current_path();
    for (int i = 0; i < 3; ++i) {
        path = path.parent_path();
    }
    return path.string();
}

std::string verifyPath(const std::string &path) {
    if (!path.empty() && path.back() != '/') {
        return path + '/';
    }
    return path;
}

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension) {
    std::vector<std::pair<std::string, std::string> > files_list_by_extension;

    for (const auto &entry: std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == file_extension) {
            std::string file_path = entry.path().string();
            std::string file_name = entry.path().filename().string();
            files_list_by_extension.emplace_back(file_path, file_name);
        }
    }

    return files_list_by_extension;
}

int get_manhattan_distance(const int cell1, const int cell2, const int n_cells_sqrt) {
    const int cell1_x = cell1 % n_cells_sqrt;
    const int cell1_y = cell1 / n_cells_sqrt;
    const int cell2_x = cell2 % n_cells_sqrt;
    const int cell2_y = cell2 / n_cells_sqrt;
    return std::abs(cell1_y - cell2_y) + std::abs(cell1_x - cell2_x);
}

//TODO
void generate_vpr_data(Graph &graph, const ReportData &data, const std::string &netPath, const std::string &placePath) {
    // Gera o nome do arquivo .net
    std::string net_name = data.dotName + "_" + data.placer + "_" + data.edgesAlgorithm + "_" +
                           std::to_string(data.execId) + ".net";
    auto nodes_idx = graph.nodesToIdx;

    // Cria e abre o arquivo .net para escrita
    std::ofstream net_file(netPath + net_name);
    if (!net_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo .net!" << std::endl;
        return;
    }

    std::vector<int> out_nodes;
    std::vector<int> pred_out_nodes;

    for (const auto &no: graph.g.nodes()) {
        if (no.find("Level") != std::string::npos && no.find("title") != std::string::npos) {
            continue;
        } else if (graph.g.out_degree(no) == 0) {
            for (const auto &pre: graph.g.predecessors(no)) {
                int pre_ = nodes_idx[pre];
                net_file << ".output out:" << pre_ << "\n";
                net_file << "pinlist:  " << pre_ << "\n\n";
                out_nodes.push_back(nodes_idx[no]);
                pred_out_nodes.push_back(nodes_idx[pre]);
            }
        } else if (graph.g.in_degree(no) == 0) {
            net_file << ".input " << nodes_idx[no] << "\n";
            net_file << "pinlist:  " << nodes_idx[no] << "\n\n";
        } else {
            net_file << ".clb " << nodes_idx[no] << "  # Only LUT used.\n";
            net_file << "pinlist:";
            int i = 0;
            for (const auto &pre: graph.g.predecessors(no)) {
                net_file << " " << nodes_idx[pre];
                ++i;
            }
            net_file << std::string(4 - i, ' ') << " open";
            net_file << " " << nodes_idx[no] << " open\n";

            net_file << "subblock: " << nodes_idx[no];
            for (int j = 0; j < i; ++j) {
                net_file << " " << j;
            }
            net_file << std::string(4 - i, ' ') << " open 4 open\n\n";
        }
    }

    // Fecha o arquivo .net automaticamente ao sair do escopo do ofstream

    // Gera o nome do arquivo .place
    std::string place_name = data.dotName + "_" + data.placer + "_" + data.edgesAlgorithm + "_" +
                             std::to_string(data.execId) + ".place";
    const std::vector<int> &placement = data.placement;
    const std::vector<int> &n2c = data.n2c;
    const auto &output_nodes = graph.output_nodes_idx;

    int grid_height = static_cast<int>(std::sqrt(data.placement.size()));
    int grid_width = grid_height;

    // Cria e abre o arquivo .place para escrita
    std::ofstream place_file(placePath + place_name);
    if (!place_file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo .place!" << std::endl;
        return;
    }

    place_file << "Netlist file: net/" << net_name << "  Architecture file: arch/k4-n1.xml\n";
    place_file << "Array size: " << (grid_width - 2) << " x " << (grid_height - 2) << " logic blocks\n";
    place_file << "#block name\tX\tY\tsubblk\tblock_number\n";
    place_file << "#----------\t--\t--\t------\t------------\n";

    int counter = 0;
    for (const auto &[node, idx]: nodes_idx) {
        int cell = n2c[idx];
        if (placement[cell] != -1) {
            // Substituto para `placement[cell] is not None`
            int x = cell % grid_width;
            int y = cell / grid_width;
            int subblk = 0;
            if (std::find(output_nodes.begin(), output_nodes.end(), idx) != output_nodes.end()) {
                auto it = std::find(out_nodes.begin(), out_nodes.end(), idx);
                int i = std::distance(out_nodes.begin(), it);
                place_file << "out:" << pred_out_nodes[i] << "\t" << x << "\t" << y << "\t" << subblk << "\t#" <<
                        counter << "\n";
            } else {
                place_file << idx << "\t" << x << "\t" << y << "\t" << subblk << "\t#" << counter << "\n";
            }
            counter++;
        }
    }
}
