#ifndef FPGA_PAR_H
#define FPGA_PAR_H

//todo Unified param header for all project and stop using parameters to choose something
//todo those parameters will be used to choose what code will be compiled

#include <string>

//PER FPGA  Paramenters File

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


//#################################################################################################
//Algorithms parameters BEGIN

//Choose the I/O qty of ports per cell to the architecture
#define IO_NUMBER  12
//TODO CLB N LUT number per cell too

//Wich algorithm will be run ********************
//Needs at least one

//Greedy algorithm that traverses the source and destination graphs once without priority
//#define FPGA_YOTO_DF

//Greedy algorithm that traverses the source and destination graphs once with priority given to the critical path.
#define FPGA_YOTO_DF_PRIO

//Greedy algorithm that traverses the source and destination graphs twice with annotations on the edges
//#define FPGA_YOTT

//Simulated annealing algorithm
//#define FPGA_SA
//***********************************************

//GREEDY algorithms parameters BRGIN ************
#if defined(FPGA_YOTO_DF) || defined(FPGA_YOTO_DF_PRIO) || defined(FPGA_YOTO_ZZ)

//Number of random search sequences for placement
#define N_DIST_VECTORS 4

//Use search strategy or not
#define STRATEGY_SEARCH

//STRATEGY SEARCH parameters BEGIN **************
#ifdef STRATEGY_SEARCH

//Wich strategy for search if STRATEGY_SEARCH is chosen
//At least one strategy is needed
#define SPIRAL_STRATEGY
//#define CURTAIN_STRATEGY

//Set the maximum search distance before using the chosen strategy
#define LIMIT_DIST 4
//#define LIMIT_DIST 5
//#define LIMIT_DIST  6
//#define LIMIT_DIST 7
//#define LIMIT_DIST 8

#endif
//STRATEGY SEARCH parameters END ****************

#endif
//GREEDY algorithms parameters EN **************

//Algorithms parameters END
//#################################################################################################

//#################################################################################################
//Rrports parameters BEGIN

//Save only the best one placement
#define BEST_ONLY

//TODO Stopped here
//Rrports parameters BEND
//#################################################################################################

//VPR version
//#define VPR_V5
#define VPR_V9

// Tests Quantity
#define RUN_1
//#define RUN_10
//#define RUN_100
//#define RUN_1000

// Benchmarks
#define TEST
//#define TRETS
//#define EPFL

//###############################

//debugging defines
#define DEBUG
//#define PRINT_DOT
#define PRINT_IMG
//*******************************

//Generate report or not

#define REPORT
#ifdef REPORT

//Choose write Make metrics reports
#define MAKE_METRICS

//Choose a type of total cost
#define FPGA_TOTAL_COST
//#define FPGA_LONG_PATH_COST

#endif
//*******************************

//Some standard variables


inline std::string benchPath = [] {
#ifdef TEST
    return "benchmarks/fpga/bench_test/";
#elif defined(TRETS)
    return "benchmarks/fpga/eval/TRETS/";
#elif defined(EPFL)
    return "benchmarks/fpga/eval/EPFL/";
#else
    return "";
#endif
}();

inline std::string algPath = [] {
    std::string path;
#ifdef TEST
    path = "/TEST";
#elifdef TRETS
    path = "/TRETS";
#elif defined(EPFL)
    path = "/EPFL";
#endif

#ifdef FPGA_YOTO_DF
    path += "/yoto_df";
#elif defined(FPGA_YOTO_DF_PRIO)
    path += "/yoto_df_prio";
#elif defined(FPGA_YOTO_ZZ)
    path += "/yoto_zz";
#elif defined(FPGA_YOTT)
    path += "/yott";
#elif defined(FPGA_YOTT_IO)
    path += "/yott_io";
#elif defined(FPGA_SA)
    path += "/sa";
#endif

#if defined(FPGA_YOTO_DF) || defined(FPGA_YOTO_DF_PRIO) || defined(FPGA_YOTO_ZZ)
#ifndef STRATEGY_SEARCH
    path += "_limit_" + std::to_string(LIMIT_DIST);
#endif
#endif

#ifdef USE_CACHE
    path += "_cache_" + std::to_string(CACHE_LINES_EXP) + "x" +
            std::to_string(CACHE_COLUMNS_EXP) + "_W_" +
            std::to_string(CACHE_W_PARAMETER) + "_" +
            std::to_string(CACHE_W_COST);
#endif

#ifdef RUN_1
    path += "_x1";
#elifdef RUN_10
    path += "_x10";
#elif defined(RUN_100)
    path += "_x100";
#elif defined(RUN_1000)
    path += "_x1000";
#endif

#ifdef BEST_ONLY
    path += "_best_only";
#endif

    path += "_ION_" + std::to_string(IO_NUMBER);

#ifdef DEBUG
    path += "_debug";
#endif

    return path;
}();

inline constexpr auto reportPath = "reports/fpga";
inline constexpr auto benchExt = ".dot";

//Execution quantity parameter
#ifdef RUN_1
inline constexpr int nExec = 1;
#elifdef RUN_10
inline constexpr int nExec = 10;
#elifdef RUN_100
inline constexpr int nExec = 100;
#elifdef RUN_1000
inline constexpr int nExec = 1000;
#endif

#endif //FPGA_PAR_H
