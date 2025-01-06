#ifndef YOTO_BASE_H
#define YOTO_BASE_H

#define CACHE_LINES_EXP 6
#define CACHE_COLUMNS_EXP 2

#include "util.h"
#include "cache.h"
#include "graph.h"
#include <vector>


ReportData yotoBase();

std::vector<std::vector<int> > getAdjCellsDist();


#endif //YOTO_BASE_H
