#ifndef PARAMETERS_H
#define PARAMETERS_H

//Choose a type of total cost
#define TOTAL_COST
//#define LONG_PATH_COST
//*******************************

//Choose if Cache will be used
//#define CACHE
//Cache Parameters
#define CACHE_LINES_EXP 6
#define CACHE_LINES (1 << CACHE_LINES_EXP)
#define CACHE_COLUMNS_EXP 2
#define CACHE_COLUMNS (1 << CACHE_COLUMNS_EXP)
//*******************************

//choose if IO will be placed - for YOTO and YOTT
//#define PLACE_IO_FIRST
//*******************************

//Choose the algorithm to be run
#define YOTO_DF
//#define YOTO_DF_PRIO
//#define YOTO_ZZ
//#define YOTT
//#define SA
//*******************************

#endif //PARAMETERS_H
