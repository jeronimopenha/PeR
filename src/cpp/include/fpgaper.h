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
};


#endif //FPGAPER_H
