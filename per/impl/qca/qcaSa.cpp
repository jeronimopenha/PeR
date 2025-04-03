#include <qca/qcaSa.h>

QcaReportData qcaSa(QCAGraph &g) {
    const string alg_type = "SA";
    int tries = 0;
    int swaps = 0;

    const int nCells = g.nCells;
    const int nCellsSqrt = g.nCellsSqrt;
    const int nNodes = g.nNodes;
    const vector<pair<int, int> > ed = g.gEdges;

    vector<int> c2n(nCells, -1);
    vector<int> n2c(nNodes, -1);

    vector<int> cells(nCells);
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);

    //place the nodes to their initial positions
    int idx = 0;
    for (int node = 0; node < nNodes; node++) {
        if (c2n[cells[idx]] == -1) {
            c2n[cells[idx]] = node;
            n2c[node] = cells[idx];
            idx++;
        }
    }

    //std::vector<std::vector<int> > neighbors = g.neighbors;

#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, nCellsSqrt);
#endif

    //begin of SA algorithm
    const float t_min = 0.001;
    float t = 100;

    auto start = chrono::high_resolution_clock::now();

    /*while (t >= t_min) {
        for (int cellA = 1; cellA < nCells; cellA++) {
            for (int cellB = 1; cellB < nCells; cellB++) {
                tries++;

                if (cellA == cellB)
                    continue;
                // Check if cellA is nor allowed, go to next
                if (fpgaIsInvalidCell(cellA, nCellsSqrt))
                    continue;
                if (fpgaIsInvalidCell(cellB, nCellsSqrt))
                    continue;

                //prevents IO nodes to be not put in IO cells
                //and put a non IO node in an IO cell
                const bool isCellAIO = fpgaIsIOCell(cellA, nCellsSqrt);
                const bool isCellBIO = fpgaIsIOCell(cellB, nCellsSqrt);

                if ((isCellAIO && !isCellBIO) || (!isCellAIO && isCellBIO))
                    continue;

                int a = c2n[cellA];
                int b = c2n[cellB];

                if (a == -1 && b == -1)
                    continue;

                int costABefore, costAAfter;
                int costBBefore, costBAfter;

                getSwapCost(n2c, a, b, cellA, cellB, nCellsSqrt, neighbors, costABefore, costAAfter, costBBefore,
                            costBAfter);

                int costAfter = costAAfter + costBAfter;
                int costBefore = costABefore + costBBefore;

                double value = exp((-1 * (costAfter - costBefore) / t));

                double rnd = dis(gen);

                if ((costAfter < costBefore) || (rnd <= value)) {
                    if (a != -1) {
                        n2c[a] = cellB;
                    }
                    if (b != -1) {
                        n2c[b] = cellA;
                    }
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
    float _time = duration.count();

    int tc = 0;
    // commented to take the cost of the longest path
#ifdef FPGA_TOTAL_COST
    tc = fpgaCalcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
#elifdef FPGA_LP_COST
    tc = calcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
#endif

    auto report = FpgaReportData(
        _time,
        g.dotName,
        g.dotPath,
        "SABase",
        cacheMisses,
        tries,
        swaps,
        alg_type,
        tc,
        c2n,
        n2c
    );
    return report;*/
}


void qcaGetSwapCost(
    const std::vector<int> &n2c,
    const int a,
    const int b,
    const int cellA,
    const int cellB,
    const int nCellsSqrt,
    const std::vector<std::vector<int> > &neighbors,
    int &costABefore,
    int &costAAfter,
    int &costBBefore,
    int &costBAfter
) {
    costABefore = 0;
    costAAfter = 0;
    costBBefore = 0;
    costBAfter = 0;

    if (a != -1) {
        for (const int node: neighbors[a]) {
            costABefore += getManhattanDist(cellA, n2c[node], nCellsSqrt);
            if (cellB == n2c[node]) {
                costAAfter += getManhattanDist(cellA, cellB, nCellsSqrt);
            } else {
                costAAfter += getManhattanDist(cellB, n2c[node], nCellsSqrt);
            }
        }
    }

    if (b != -1) {
        for (const int node: neighbors[b]) {
            costBBefore += getManhattanDist(cellB, n2c[node], nCellsSqrt);
            if (cellA == n2c[node]) {
                costBAfter += getManhattanDist(cellB, cellA, nCellsSqrt);
            } else {
                costBAfter += getManhattanDist(cellA, n2c[node], nCellsSqrt);
            }
        }
    }
}
