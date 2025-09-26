#ifndef FPGA_PAR_H
#define FPGA_PAR_H

//todo Unified param header for all project and stop using parameters to choose something
//todo those parameters will be used to choose what code will be compiled

#include <common/definitions.h>

//CACHE definitions on cachePar.h file


//PER FPGA  Paramenters File
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

//#################################################################################################
//Algorithms parameters BEGIN

//Choose the I/O qty of ports per cell to the architecture
#define IO_NUMBER  8
//TODO CLB N LUT number per cell too

//Wich algorithm will be run ********************
//Needs at least one

//fixme this algorithm was used with priority. Needed to change the code to use the correct one
//Greedy algorithm that traverses the source and destination graphs once without priority
//with depth first search
//#define FPGA_YOTO_DF

//Greedy algorithm that traverses the source and destination graphs once with priority given to the critical path.
//with depth first search
#define FPGA_YOTO_DF_PRIO

//Greedy algorithm that traverses the source and destination graphs once without priority
//with zigzag search
//#define FPGA_YOTO_ZZ

//Greedy algorithm that traverses the source and destination graphs twice with annotations on the edges
//#define FPGA_YOTT

//Simulated annealing algorithm
//#define FPGA_SA
//***********************************************

//GREEDY algorithms parameters BRGIN ************
#if defined(FPGA_YOTO_DF) || defined(FPGA_YOTO_DF_PRIO) || defined(FPGA_YOTO_ZZ)

//Number of random search sequences for placement
#define N_DIST_VECTORS 4

//Use search strategies or not
#define STRATEGY_SEARCH

//STRATEGY SEARCH parameters BEGIN **************
#ifdef STRATEGY_SEARCH

//Wich strategy for search if STRATEGY_SEARCH is chosen
//At least one strategy is needed
//#define SPIRAL_STRATEGY
//#define SCAN_STRATEGY

#define IO_STRATEGY

#ifdef SCAN_STRATEGY
#define SCAN_QUADRANTS 16
#endif

//#define LIMIT_STRATEGY

#ifdef LIMIT_STRATEGY
//Set the maximum search distance before using the chosen strategy
#define LIMIT_DIST 2
//#define LIMIT_DIST 3
//#define LIMIT_DIST 4
//#define LIMIT_DIST 5
//#define LIMIT_DIST  6
//#define LIMIT_DIST 7
//#define LIMIT_DIST 8
#endif

#endif
//STRATEGY SEARCH parameters END ****************

#endif
//GREEDY algorithms parameters END **************

//Algorithms parameters END
//#################################################################################################

//#################################################################################################
//Reports parameters BEGIN

//Generate report or not

#define REPORT_PREFIX ""

//Choose write Make metrics reports
#define MAKE_METRICS

//Choose a type of total cost
//#define FPGA_TOTAL_COST
#define FPGA_DISTANCE_SLACK_COST


//Save only the best one placement
//#define BEST_ONLY

//VPR version
//fixme need to debug if the vpr5 reports are still working
//#define VPR_V5
#define VPR_V9

//Reports parameters END
//#################################################################################################

//#################################################################################################
//Execution parameters Begin

// Tests Quantity
#define RUN_1
//#define RUN_6
//#define RUN_60
//#define RUN_600
//#define RUN_10
//#define RUN_100
//#define RUN_1000

// Benchmarks
#define TEST
//#define TRETS
//#define EPFL

//Debugging *************************************
//debugging defines
#define DEBUG

//#define PRINT_DOT
#ifdef PRINT_DOT
#define DOT_PATH  "/home/jeronimo/tmp/placed.dot"
#endif


#define PRINT_IMG
#ifdef PRINT_IMG
#define JPG_PATH  "/home/jeronimo/tmp/placed.jpg"
#endif
//*******************************

#define TOTAL_SNAPSHOTS 10

//Execution parameters END
//#################################################################################################


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
    path = "/TEST/";
#elifdef TRETS
    path = "/TRETS/";
#elif defined(EPFL)
    path = "/EPFL/";
#endif

#ifdef REPORT_PREFIX
    path += REPORT_PREFIX;
#endif


#ifdef FPGA_YOTO_DF
    path += "yoto_df";
#elif defined(FPGA_YOTO_DF_PRIO)
    path += "yoto_df_prio";
#elif defined(FPGA_YOTO_ZZ)
    path += "yoto_zz";
#elif defined(FPGA_YOTT)
    path += "yott";
#elif defined(FPGA_YOTT_IO)
    path += "yott_io";
#elif defined(FPGA_SA)
    path += "sa";
#endif

#if defined(FPGA_YOTO_DF) || defined(FPGA_YOTO_DF_PRIO) || defined(FPGA_YOTO_ZZ)
#ifdef LIMIT_STRATEGY
    path += "_limit_" + std::to_string(LIMIT_DIST);
#endif

#ifdef SCAN_STRATEGY
    path += "_scan";
#endif

#ifdef FPGA_TOTAL_COST
    path += "_TC";
#elifdef FPGA_LONG_PATH_COST
    path += "_LPC";
#elifdef FPGA_DISTANCE_SLACK_COST
    path += "_DSC";
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
#elif defined(RUN_6)
    path += "_x6";
#elif defined(RUN_60)
    path += "_x60";
#elif defined(RUN_600)
    path += "_x600";
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

//Execution quantity parameter
#ifdef RUN_1
inline constexpr int nExec = 1;
#elifdef RUN_10
inline constexpr int nExec = 10;
#elifdef RUN_100
inline constexpr int nExec = 100;
#elifdef RUN_1000
inline constexpr int nExec = 1000;
#elifdef RUN_6
inline constexpr int nExec = 6;
#elifdef RUN_60
inline constexpr int nExec = 60;
#elifdef RUN_600
inline constexpr int nExec = 600;
#endif

#ifdef FPGA_TOTAL_COST
inline const std::string costStrategyName = STRINGIFY(FPGA_TOTAL_COST);
#elifdef FPGA_LONG_PATH_COST
inline const std::string costStrategyName = STRINGIFY(FPGA_LONG_PATH_COST);
#elifdef FPGA_DISTANCE_SLACK_COST
inline const std::string costStrategyName = STRINGIFY(FPGA_DISTANCE_SLACK_COST);
#else
inline const std::string costStrategyName;
#endif

inline constexpr auto reportPath = "reports/fpga";
inline constexpr auto benchExt = ".dot";

#endif //FPGA_PAR_H
