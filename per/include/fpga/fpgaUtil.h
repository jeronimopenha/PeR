#ifndef FPGA_UTIL_H
#define FPGA_UTIL_H

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <common/definitions.h>
#include <common/cache.h>
#include <fpga/fpgaPar.h>
#include <fpga/fpgaGraph.h>

struct RGB {
    int r, g, b;
};

struct FpgaReportData {
    double _time = 0.0;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    long size = 0;
    long nNodes = 0;
    long nIOs = 0;
    long nIOpCell = IO_NUMBER;
    std::string edgesAlgorithm;
    std::string costStrategy;
    long totalCost = 0;
    std::vector<std::vector<long> > c2n;
    std::vector<std::pair<long, long> > n2c;
#ifdef MAKE_METRICS
#ifdef USE_CACHE
    long cacheMisses = -1;
    long w = 0;
    long wCost = 0;
    long cachePenalties = -1;
#endif
    long clbTries = 0;
    long ioTries = 0;
    long tries = 0;
#ifdef USE_CACHE
    long triesP = 0;
#endif
    long triesPerNode = 0;
    long swaps = 0;
    std::vector<std::map<long, long> > hist;
    std::vector<long> heatEnd;
    std::vector<long> heatBegin;
    std::map<long, std::vector<long> > orDest;
#endif

    FpgaReportData();

    FpgaReportData(
        double _time,
        std::string dotName,
        std::string dotPath,
        std::string placer,
        long size,
        long nNodes,
        long nIOs,
        std::string edgesAlgorithm,
        std::string costStrategy,
        long totalCost,
        const std::vector<std::vector<long> > &c2n,
        const std::vector<std::pair<long, long> > &n2c
#ifdef MAKE_METRICS
        ,
#ifdef USE_CACHE
        long cacheMisses,
        long w,
        long wCost,
        long cachePenalties,
#endif
        long clbTries,
        long ioTries,
        long tries,
#ifdef USE_CACHE
        long triesP,
#endif
        long triesPerNode,
        long swaps,
        std::vector<std::map<long, long> > hist,
        std::vector<long> heatEnd,
        std::vector<long> heatBegin,
        std::map<long, std::vector<long> > orDest
#endif
    );

    [[nodiscard]] std::string to_json() const;

    [[nodiscard]] std::string metrics_to_json() const;
};

enum class QuadrantDirection {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};


long fpgaMinBorderDist(long cell, long nCellsSqrt);


void fpgaWriteReports(const std::string &basePath,
                      const std::string &fileName,
                      const FpgaReportData &data);

void fpgaWriteVpr5Data(const std::string &basePath,
                       const std::string &fileName,
                       const FpgaReportData &data,
                       FPGAGraph g);

void fpgaWriteVpr9Data(const std::string &basePath,
                       const std::string &fileName,
                       const FpgaReportData &data,
                       FPGAGraph g);

bool fpgaIsInvalidCell(long line, long column, long nCellsSqrt);

bool fpgaIsIOCell(long line, long column, long nCellsSqrt);

long getQuadrant(long line, long column, long nCellsSqrt);

std::vector<std::pair<long, int> > getAdjacentQuadrants(long q);

#ifdef PRINT_DOT
void fpgaSavePlacedDot(std::vector<std::pair<long, long> > &n2c, std::vector<std::vector<long> > &c2n,
                       const std::vector<std::pair<long, long> > &ed, long nCellsSqrt);
#endif

#ifdef PRINT_IMG
void writeMap(const std::vector<std::vector<long> > &c2n, const std::pair<long, long> &lastPlaced, long nCellsSqrt,
              const std::string &filePath);
#endif
std::vector<std::vector<long> > fpgaGetDistVectors(long nCellsSqrt);

#ifdef MAKE_METRICS
long fpgaCalcGraphTotalDistance(const std::vector<std::pair<long, long> > &n2c,
                                const std::vector<std::pair<long, long> > &edges,
                                long nCellsSqrt);
#endif


/*long fpgaCalcGraphLPDistance(const std::vector<long> &longestPath, const std::vector<long> &n2c,
                             long nCellsSqrt);*/


/*
RGB valueToRGB(float normValue);
*/


/*
void writeHeatmap(const std::vector<long> &heatData,
                  const std::vector<std::vector<long> > &c2n,
                  long nCellsSqrt,
                  const std::string &basePath,
                  const std::string &reportPath,
                  const std::string &algPath,
                  const std::string &fileName,
                  const std::string &suffix);
                  */

/*void drawChar(std::vector<unsigned char> &image,
              int imgWidth,
              int x,
              int y,
              char c,
              int scale = 1);

void drawText(std::vector<unsigned char> &image,
              int imgWidth,
              int x,
              int y,
              const std::string &text,
              int scale = 1);

void writeHist(const std::map<long, long> &hist,
               const std::string &basePath,
               const std::string &reportPath,
               const std::string &algPath,
               const std::string &fileName,
               const std::string &suffix);

void writeBoxplot(const std::map<long, long> &hist,
                  const std::string &basePath,
                  const std::string &reportPath,
                  const std::string &algPath,
                  const std::string &fileName,
                  const std::string &suffix);

//fonts in pixel map
const std::map<char, std::vector<std::string> > font5x7 = {
    {
        '0', {
            " ### ",
            "#   #",
            "#  ##",
            "# # #",
            "##  #",
            "#   #",
            " ### "
        }
    },
    {
        '1', {
            "  #  ",
            " ##  ",
            "# #  ",
            "  #  ",
            "  #  ",
            "  #  ",
            "#####"
        }
    },
    {
        '2', {
            " ### ",
            "#   #",
            "    #",
            "   # ",
            "  #  ",
            " #   ",
            "#####"
        }
    },
    {
        '3', {
            " ### ",
            "#   #",
            "    #",
            " ### ",
            "    #",
            "#   #",
            " ### "
        }
    },
    {
        '4', {
            "   # ",
            "  ## ",
            " # # ",
            "#  # ",
            "#####",
            "   # ",
            "   # "
        }
    },
    {
        '5', {
            "#####",
            "#    ",
            "#    ",
            "#### ",
            "    #",
            "#   #",
            " ### "
        }
    },
    {
        '6', {
            " ### ",
            "#   #",
            "#    ",
            "#### ",
            "#   #",
            "#   #",
            " ### "
        }
    },
    {
        '7', {
            "#####",
            "    #",
            "   # ",
            "  #  ",
            "  #  ",
            "  #  ",
            "  #  "
        }
    },
    {
        '8', {
            " ### ",
            "#   #",
            "#   #",
            " ### ",
            "#   #",
            "#   #",
            " ### "
        }
    },
    {
        '9', {
            " ### ",
            "#   #",
            "#   #",
            " ####",
            "    #",
            "#   #",
            " ### "
        }
    },
    {
        '-', {
            "     ",
            "     ",
            "     ",
            " ### ",
            "     ",
            "     ",
            "     "
        }
    },
    {
        '.', {
            "     ",
            "     ",
            "     ",
            "     ",
            "     ",
            "  ## ",
            "  ## "
        }
    },
    {
        ' ', {
            "     ",
            "     ",
            "     ",
            "     ",
            "     ",
            "     ",
            "     "
        }
    }
};*/

#endif
