//
// Created by jeronimo on 02/11/24.
//

#include "fpgaper.h"

#include <utility>

FPGAPPeR::FPGAPPeR(const Graph &graph): graph(graph) {
    getInOutPos();
}

void FPGAPPeR::perYoto(int nExec) {
    for (int i = 0; i < nExec; i++) {
        std::vector<int> placement(graph.nCells, -1);
        std::vector<int> n2c(graph.nNodes, -1);
        std::vector<std::vector<std::vector<int> > > distCells = graph.getMeshDistances();

        int tries = 0;
        int swaps = 0;

        std::vector<std::pair<std::string, std::string> > edStr = graph.getEdgesDepthFirst();
        std::vector<std::pair<int, int> > ed = graph.getEdgesIdx(edStr);
        std::vector<int> nodes = graph.getNodesIdx((graph.inputNodesStr));

        placeNodes(n2c, placement, possibleInOut, nodes);


        for (auto [a,b]: ed) {
            if (n2c[b] != 1) {
                continue;
            }
            if (n2c[a] == -1) {
            }
        }
    }
}

void FPGAPPeR::getInOutPos() {
    // Append positions in the first range
    for (int i = 1; i < graph.nCellsSqrt - 1; ++i) {
        possibleInOut.push_back(i);
    }

    // Append positions in the second range
    for (int i = 1; i < graph.nCellsSqrt - 1; ++i) {
        possibleInOut.push_back(i + graph.nCells - graph.nCellsSqrt);
    }

    // Append positions in the third range
    for (int i = graph.nCellsSqrt; i < graph.nCells - graph.nCellsSqrt; i += graph.nCellsSqrt) {
        possibleInOut.push_back(i);
    }

    // Append positions in the fourth range
    for (int i = graph.nCellsSqrt * 2 - 1; i < graph.nCells - 1; i += graph.nCellsSqrt) {
        possibleInOut.push_back(i);
    }
    int a = 1;
}


void FPGAPPeR::placeNodes(std::vector<int> &n2c, std::vector<int> &placement, const std::vector<int> &possible_pos,
                          const std::vector<int> &nodes) {
    int i = 0;
    while (i < nodes.size()) {
        const int n = nodes[i];
        while (true) {
            if (const int ch = choose_position(placement, possible_pos); placement[ch] == -1) {
                placement[ch] = n;
                n2c[n] = ch;
                break;
            }
        }
        i++;
    }
}

int FPGAPPeR::choose_position(const std::vector<int> &placement, const std::vector<int> &choices) {
    while (true) {
        auto it = std::next(choices.begin(), rand() % choices.size()); // Randomly select an element from the set
        if (const int ch = *it; placement[ch] == -1) {
            return ch;
        }
    }
}
