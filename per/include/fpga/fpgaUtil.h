#ifndef FPGA_UTIL_H
#define FPGA_UTIL_H

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <algorithm>
#include <fpga/fpgaGraph.h>
#include <common/util.h>
#include <map>

struct RGB {
    int r, g, b;
};

struct FpgaReportData {
    double _time = 0.0;
    std::string dotName;
    std::string dotPath;
    std::string placer;
    long cacheMisses = 0;
    long w = 0;
    long wCost = 0;
    long cachePenalties = 0;
    long clbTries = 0;
    long ioTries = 0;
    long tries = 0;
    long triesP = 0;
    long swaps = 0;
    std::string edgesAlgorithm;
    long totalCost = 0;
    long lPCost = 0;
    std::vector<long> c2n;
    std::vector<long> n2c;
    std::vector<std::map<long, long> > hist;
    std::vector<long> heatEnd;
    std::vector<long> heatBegin;

    FpgaReportData();

    FpgaReportData(double _time, std::string dotName, std::string dotPath, std::string placer, long cacheMisses, long w,
                   long wCost, long cachePenalties, long clbTries, long ioTries, long tries, long triesP, long swaps,
                   std::string edges_algorithm, long totalCost, long lPCost, const std::vector<long> &c2n,
                   const std::vector<long> &n2c, std::vector<std::map<long, long> > hist, std::vector<long> heatEnd,
                   std::vector<long> heatBegin);

    [[nodiscard]] std::string to_json() const;
};

void fpgaSavePlacedDot(std::vector<long> &n2c, const std::vector<std::pair<long, long> > &ed, long nCellsSqrt,
                       const std::string &filename);

std::vector<std::vector<long> > fpgaGetAdjCellsDist(long nCellsSqrt);

long fpgaCalcGraphTotalDistance(const std::vector<long> &n2c, const std::vector<std::pair<long, long> > &edges,
                                long nCellsSqrt);

long fpgaCalcGraphLPDistance(const std::vector<long> &longestPath, const std::vector<long> &n2c,
                             long nCellsSqrt);

bool fpgaIsInvalidCell(long cell, long nCellsSqrt);

bool fpgaIsIOCell(long cell, long nCellsSqrt);

long fpgaMinBorderDist(long cell, long nCellsSqrt);

void fpgaWriteJson(const std::string &basePath,
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

RGB valueToRGB(const float normValue);

void writeHeatmap(const std::vector<long> &heatData,
                  const std::vector<long> &c2n,
                  long nCellsSqrt,
                  const std::string &basePath,
                  const std::string &reportPath,
                  const std::string &algPath,
                  const std::string &fileName,
                  const std::string &suffix);

void writeHist(const std::map<long, long> &hist,
               const std::string &basePath,
               const std::string &reportPath,
               const std::string &algPath,
               const std::string &fileName,
               const std::string &suffix);

void writeBoxplot(const std::map<long, long>& hist,
                  const std::string& basePath,
                  const std::string& reportPath,
                  const std::string& algPath,
                  const std::string& fileName,
                  const std::string& suffix);

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
