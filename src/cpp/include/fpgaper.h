//
// Created by jeronimo on 02/11/24.
//

#ifndef FPGAPER_H
#define FPGAPER_H
#include "graph.h"


class FPGAPPeR {
public:
    Graph graph;
    std::vector<int> possibleInOut;


    FPGAPPeR(const Graph &graph);

    void getInOutPos();

    void perYoto(int nExec);

    void placeNodes(std::vector<int> &n2c, std::vector<int> &placement, const std::vector<int> &possible_pos,
                     const std::vector<int> &nodes);
    int FPGAPPeR::choose_position(const std::vector<int>& placement, const std::vector<int>& choices);
};


#endif //FPGAPER_H
