#include <fpga/fpgaSa.h>
#include <fpga/fpgaPar.h>

using namespace std;

FpgaReportData fpgaSa(FPGAGraph &g) {
    const string alg_type = "SA";
    long cacheMisses = 0;
    long tries = 0;
    long swaps = 0;

    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;
    const vector<pair<long, long> > ed = g.gEdges;


    vector<long> c2n(nCells, -1);
    vector<long> n2c(nNodes, -1);

    vector<long> inOutCells = g.getInOutPos();
    randomVector(inOutCells);

    vector<long> clbCells = g.getClbPos();
    randomVector(clbCells);

    //place the io nodes to their initial positions
    vector<long> ioNodes;
    ioNodes.reserve(g.inputNodes.size() + g.outputNodes.size());
    ioNodes.insert(ioNodes.end(), g.inputNodes.begin(), g.inputNodes.end());
    ioNodes.insert(ioNodes.end(), g.outputNodes.begin(), g.outputNodes.end());


    long idx = 0;
    for (auto node: ioNodes) {
        if (c2n[inOutCells[idx]] == -1) {
            c2n[inOutCells[idx]] = node;
            n2c[node] = inOutCells[idx];
            idx++;
        }
    }

    //place the clb nodes to their initial positions
    vector<long> clbNodes = g.outputNodes;

    idx = 0;
    for (auto node: clbNodes) {
        if (c2n[clbCells[idx]] == -1) {
            c2n[clbCells[idx]] = node;
            n2c[node] = clbCells[idx];
            idx++;
        }
    }

    vector<vector<long> > neighbors = g.neighbors;

#ifdef DEBUG
    fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

    //begin of SA algorithm
    constexpr float t_min = 0.001f;
    float t = 100;

    auto start = chrono::high_resolution_clock::now();

    while (t >= t_min) {
        for (int cellA = 1; cellA < nCells; cellA++) {
            for (int cellB = 1; cellB < nCells; cellB++) {
                tries++;
                const long lA = cellA / nCellsSqrt;
                const long cA = cellA % nCellsSqrt;
                const long lB = cellB / nCellsSqrt;
                const long cB = cellB % nCellsSqrt;

                if (cellA == cellB)
                    continue;
                // Check if cellA is nor allowed, go to next
                if (fpgaIsInvalidCell(lA, cA, nCellsSqrt))
                    continue;
                if (fpgaIsInvalidCell(lB, cB, nCellsSqrt))
                    continue;

                //prevents IO nodes to be not put in IO cells
                //and put a non IO node in an IO cell
                const bool isCellAIO = fpgaIsIOCell(lA, cA, nCellsSqrt);
                const bool isCellBIO = fpgaIsIOCell(lB, cB, nCellsSqrt);

                if ((isCellAIO && !isCellBIO) || (!isCellAIO && isCellBIO))
                    continue;

                long a = c2n[cellA];
                long b = c2n[cellB];

                if (a == -1 && b == -1)
                    continue;

                long costABefore, costAAfter;
                long costBBefore, costBAfter;

                fpgaGetSwapCost(n2c, a, b, cellA, cellB, nCellsSqrt, neighbors, costABefore, costAAfter, costBBefore,
                                costBAfter);

                long costAfter = costAAfter + costBAfter;
                long costBefore = costABefore + costBBefore;

                double value = exp((-1.0 * static_cast<double>(costAfter - costBefore) / t));

                const double rnd = randomFloat(0.0f, 1.0f);

                if ((costAfter < costBefore) || (rnd <= value)) {
                    if (a != -1)
                        n2c[a] = cellB;
                    if (b != -1)
                        n2c[b] = cellA;
                    c2n[cellA] = b;
                    c2n[cellB] = a;
                    swaps++;
#ifdef DEBUG
                    //savePlacedDot(n2c, ed, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
                }
            }
            t *= 0.999;
        }
    }

#ifdef DEBUG
    fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    auto _time = duration.count();

    long tc = 0;
    // commented to take the cost of the longest path
#ifdef FPGA_TOTAL_COST
    tc = fpgaCalcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
#elifdef FPGA_LP_COST
    tc = calcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
#endif

    /*auto report = FpgaReportData(
        _time,
        g.dotName,
        g.dotPath,
        "SABase",
        cacheMisses,
        0,0,0,
        0,0,
        tries,
        0,
        swaps,
        alg_type,
        tc,
        0,
        c2n,
        n2c
    );*/
    return FpgaReportData();
}


void fpgaGetSwapCost(
    const std::vector<long> &n2c,
    const long a,
    const long b,
    const long cellA,
    const long cellB,
    const long nCellsSqrt,
    const std::vector<std::vector<long> > &neighbors,
    long &costABefore,
    long &costAAfter,
    long &costBBefore,
    long &costBAfter
) {
    costABefore = 0;
    costAAfter = 0;
    costBBefore = 0;
    costBAfter = 0;

    if (a != -1) {
        for (const auto node: neighbors[a]) {
            costABefore += getManhattanDist(cellA, n2c[node], nCellsSqrt);
            if (cellB == n2c[node]) {
                costAAfter += getManhattanDist(cellA, cellB, nCellsSqrt);
            } else {
                costAAfter += getManhattanDist(cellB, n2c[node], nCellsSqrt);
            }
        }
    }

    if (b != -1) {
        for (const auto node: neighbors[b]) {
            costBBefore += getManhattanDist(cellB, n2c[node], nCellsSqrt);
            if (cellA == n2c[node]) {
                costBAfter += getManhattanDist(cellB, cellA, nCellsSqrt);
            } else {
                costBAfter += getManhattanDist(cellA, n2c[node], nCellsSqrt);
            }
        }
    }
}
