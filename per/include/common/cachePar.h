#ifndef CACHE_PAR_H
#define CACHE_PAR_H

//defines for all project
#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <random>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>

//#################################################################################################
//CACHE definitions BEGIN

//The code needs to simulate a cache or not
//#define USE_CACHE

//CACHE Parameters ******************************
#ifdef USE_CACHE
#define CACHE_LINES_EXP 10
#define CACHE_LINES (1 << CACHE_LINES_EXP)
#define CACHE_COLUMNS_EXP 2
#define CACHE_COLUMNS (1 << CACHE_COLUMNS_EXP)
#define CACHE_W_PARAMETER 8
#define CACHE_W_COST 10
#endif
//***********************************************

//CACHE definitions END
//#################################################################################################


#endif //CACHE_PAR_H
