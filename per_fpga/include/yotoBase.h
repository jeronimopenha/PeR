#ifndef YOTO_BASE_H
#define YOTO_BASE_H


#include "util.h"
#include "graph.h"
#include <vector>

ReportData yotoBase(yotoAlgEnum alg = DF);

std::vector<std::vector<int> > getAdjCellsDist();

#endif //YOTO_BASE_H
