#ifndef FPGA_PAR_H
#define FPGA_PAR_H

//todo Unified param header for all project and stop using parameters to choose something
//todo those parameters will be used to choose what code will be compiled

#include <string>

//PER FPGA PARAMETERS

//Choose the I/O qty of ports per cell
#define IO_NUMBER  13

//Choose if Cache will be used
//#define CACHE
// Parameters
#define CACHE_LINES_EXP 10
#define CACHE_LINES (1 << CACHE_LINES_EXP)
#define CACHE_COLUMNS_EXP 2
#define CACHE_COLUMNS (1 << CACHE_COLUMNS_EXP)
#define CACHE_W_PARAMETER 8
#define CACHE_W_COST 10
//*******************************

//Save only the best one
#define BEST_ONLY

//Choose the algorithm to be run
#define FPGA_YOTO_DF
//#define FPGA_YOTO_DF_PRIO
//#define FPGA_YOTO_ZZ
//#define FPGA_YOTT
//#define FPGA_SA
//*******************************

#if defined(FPGA_YOTO_DF) || defined(FPGA_YOTO_DF_PRIO) || defined(FPGA_YOTO_ZZ)
#define N_DIST_VECTORS 4

#define UNLIMITED_DIST

#ifndef UNLIMITED_DIST
#define LIMIT_DIST 4
//#define LIMIT_DIST 5
//#define LIMIT_DIST  6
//#define LIMIT_DIST 7
//#define LIMIT_DIST 8
#endif
#endif

//VPR version
//#define VPR_V5
#define VPR_V9

// Tests Quantity
//#define RUN_1
//#define RUN_10
//#define RUN_100
#define RUN_1000

// Benchmarks
#define TEST
//#define TRETS
//#define EPFL

//###############################

//debugging defines
//#define DEBUG
//#define PRINT
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
#ifndef UNLIMITED_DIST
    path += "_limit_" + std::to_string(LIMIT_DIST);
#endif
#endif

#ifdef CACHE
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
