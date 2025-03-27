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
#define PLACE_IO_FIRST
//*******************************

//todo debugged only the YOTO_DF with and without placeiofirst
//todo debugged the yoto without placeiofirst. with the placeio i need to add annotations]
//todo the othrt algorithms need to me debugged yet

//Choose the algorithm to be run
#define YOTO_DF
//#define YOTO_DF_PRIO
//#define YOTO_ZZ
//#define YOTT
//#define SA
//*******************************


//debugging defines
//#define DEBUG
//*******************************
#endif //PARAMETERS_H
