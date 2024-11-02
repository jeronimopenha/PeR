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
