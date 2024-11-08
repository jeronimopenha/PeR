//
// Created by jeronimo on 02/11/24.
//

#ifndef FPGAPER_H
#define FPGAPER_H
#include "graph.h"
#include "util.h"


class FPGAPPeR {
public:
    Graph graph;
    std::vector<int> possibleInOut;


    FPGAPPeR(const Graph &graph);

    void getInOutPos();

    void placeNodes(std::vector<int> &n2c, std::vector<int> &placement, const std::vector<int> &possible_pos,
                    const std::vector<int> &nodes);

    static int choosePosition(const std::vector<int> &placement, const std::vector<int> &choices);

    int calcTotalDistance(const std::vector<int> &n2c, const std::vector<std::pair<int, int> > &edges);

    std::unordered_map<int, ReportData> FPGAPPeR::perSa(int nExec);

    std::unordered_map<int, ReportData> perYoto(int nExec);

    std::unordered_map<int, ReportData> FPGAPPeR::perYott(int nExec);
};


#endif //FPGAPER_H
