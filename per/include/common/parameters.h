#ifndef PARAMETERS_H
#define PARAMETERS_H

//PER FPGA PARAMETERS
//Choose a type of total cost
#define FPGA_TOTAL_COST
//#define FPGA_LONG_PATH_COST
//*******************************

//Choose if Cache will be used
//#define CACHE
// Parameters
#define CACHE_LINES_EXP 10
#define CACHE_LINES (1 << CACHE_LINES_EXP)
#define CACHE_COLUMNS_EXP 2
#define CACHE_COLUMNS (1 << CACHE_COLUMNS_EXP)
//*******************************

//Choose the algorithm to be run

//#define FPGA_YOTO_DF
//#define FPGA_YOTO_DF_PRIO
//#define FPGA_YOTO_ZZ
#define FPGA_YOTT
//#define FPGA_SA
//*******************************

//#define TRETS
#define EPFL
//###############################


//PER QCA PARAMETERS
#define  MAX_EXTRA_LAYERS 5
#define EXPLORE_FACTOR 1

//QCA Tecnology
//#define USE
#define WAVE2D

//Choose the algorithm to be run
//#define QCA_YOTO_ZZ
#define QCA_YOTT
//#define QCA_SA
//###############################

//debugging defines
//#define DEBUG
//#define PRINT
//*******************************
#endif //PARAMETERS_H
