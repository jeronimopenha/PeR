#ifndef FPGA_UTIL_H
#define FPGA_UTIL_H

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <algorithm>
#include <fpga/fpgaGraph.h>
#include <common/util.h>
#include <map>
#include <fpga/fpgaPar.h>

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
    long cacheMisses = -1;
    long w = 0;
    long wCost = 0;
    long cachePenalties = -1;
    long clbTries = 0;
    long ioTries = 0;
    long tries = 0;
    long triesP = 0;
    long triesPerNode = 0;
    long swaps = 0;
    std::string edgesAlgorithm;
    long totalCost = 0;
    long lPCost = 0;
    std::vector<std::vector<long> > c2n;
    std::vector<std::pair<long, long> > n2c;
    std::vector<std::map<long, long> > hist;
    std::vector<long> heatEnd;
    std::vector<long> heatBegin;
    std::map<long, std::vector<long> > orDest;

    FpgaReportData();

    FpgaReportData(double _time, std::string dotName, std::string dotPath, std::string placer, long size, long nNodes,
                   long nIOs, long cacheMisses, long w, long wCost, long cachePenalties, long clbTries, long ioTries,
                   long tries, long triesP, long triesPerNode, long swaps, std::string edges_algorithm,
                   long totalCost, long lPCost, const std::vector<std::vector<long> > &c2n,
                   const std::vector<std::pair<long, long> > &n2c, std::vector<std::map<long, long> > hist,
                   std::vector<long> heatEnd, std::vector<long> heatBegin, std::map<long, std::vector<long> >);

    [[nodiscard]] std::string to_json() const;

    [[nodiscard]] std::string metrics_to_json() const;
};

void fpgaSavePlacedDot(std::vector<std::pair<long, long> > &n2c, std::vector<std::vector<long> > &c2n,
                       const std::vector<std::pair<long, long> > &ed, const long nCellsSqrt,
                       const std::string &filename);

std::vector<std::vector<long> > fpgaGetAdjCellsDist(long nCellsSqrt);

long fpgaCalcGraphTotalDistance(const std::vector<long> &n2c, const std::vector<std::pair<long, long> > &edges,
                                long nCellsSqrt);

long fpgaCalcGraphLPDistance(const std::vector<long> &longestPath, const std::vector<long> &n2c,
                             long nCellsSqrt);

long fpgaMinBorderDist(long cell, long nCellsSqrt);


void fpgaWriteReports(const std::string &basePath,
                      const std::string &reportPath,
                      const std::string &algPath,
                      const std::string &fileName,
                      const FpgaReportData &data);

void fpgaWriteVprData(const std::string &basePath,
                      const std::string &reportPath,
                      const std::string &algPath,
                      const std::string &fileName,
                      const FpgaReportData &data,
                      FPGAGraph g);

bool fpgaIsInvalidCell(long l, long c, long nCellsSqrt);

bool fpgaIsIOCell(long l, long c, long nCellsSqrt);

RGB valueToRGB(float normValue);

void writeHeatmap(const std::vector<long> &heatData,
                  const std::vector<long> &c2n,
                  long nCellsSqrt,
                  const std::string &basePath,
                  const std::string &reportPath,
                  const std::string &algPath,
                  const std::string &fileName,
                  const std::string &suffix);

void drawChar(std::vector<unsigned char> &image,
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
};

#endif
