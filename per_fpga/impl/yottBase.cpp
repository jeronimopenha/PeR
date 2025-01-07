#include "yottBase.h"

ReportData yottBase(Graph &g) {
    int nCells = g.nCells;
    int nCellsSqrt = g.nCellsSqrt;
    int nNodes = g.nNodes;

    std::vector<int> c2n(nCells, -1);
    std::vector<int> n2c(nNodes, -1);
    std::vector<std::vector<int> > distCells = getAdjCellsDist(nCellsSqrt);
    std::vector<int> inOutCells = g.getInOutPos();

    randomVector(inOutCells);

    int cacheMisses = 0;
    int tries = 0;
    int swaps = 0;

    std::string alg_type = "ZIG_ZAG";
    std::vector<std::pair<int, int> > ed;
    std::vector<std::pair<int, int> > convergence;
    ed = g.getEdgesZigzag(convergence);

    //saveToDot(ed, "/home/jeronimo/test.dot");
    int lastIdxIOCellUsed = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (auto [a,b]: ed) {
        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks


        if (n2c[a] == -1) {
            for (int i = lastIdxIOCellUsed + 1; i < inOutCells.size(); i++) {
                int ioCell = inOutCells[i];

                if (c2n[ioCell] == -1) {
                    c2n[ioCell] = a;
                    n2c[a] = ioCell;
                    lastIdxIOCellUsed = i;
                    break;
                }
            }
        }

        //Now, if B is placed, go to next edge

        if (n2c[b] != -1) {
            continue;
        }

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const int lA = n2c[a] / nCellsSqrt;
        const int cA = n2c[a] % nCellsSqrt;

        //bool placed = false;
        //Then I will look for a cell next to A's cell
        for (const auto &ij: distCells) {
            ++tries;
            const int lB = lA + ij[0];
            const int cB = cA + ij[1];

            // Define boundary and corner conditions
            const bool outOfBounds = (lB < 0 || lB >= nCellsSqrt || cB < 0 || cB >= nCellsSqrt);
            const bool isTopLeftCorner = (lB == 0 && cB == 0);
            const bool isBottomRightCorner = (lB == nCellsSqrt - 1 && cB == nCellsSqrt - 1);
            const bool isBottomLeftCorner = (lB == nCellsSqrt - 1 && cB == 0);
            const bool isTopRightCorner = (lB == 0 && cB == nCellsSqrt - 1);
            const bool isCorner = isTopLeftCorner || isTopRightCorner || isBottomLeftCorner || isBottomRightCorner;

            // Check if the target cell is nor allowed, go to next
            if (outOfBounds || isCorner) {
                continue;
            }

            //find the idx for the target cell
            int targetCell = lB * nCellsSqrt + cB;

            const bool isTargetCellIO = lB == 0 || lB == nCellsSqrt - 1 || cB == 0 || cB == nCellsSqrt - 1;
            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            //prevents IO nodes to be not put in IO cells
            //and put a non IO noce in an IO cell
            if (isTargetCellIO) {
                // 'targetCell' is a IO cell
                if (!IsBIoNode) {
                    continue;
                }
            } else {
                // 'targetCell' is not in possible_positions
                if (IsBIoNode) {
                    continue;
                }
            }

            // Place the node if `placement[targetCell]` is unoccupied

            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                //placed = true;
                break;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    float _time = duration.count();
    int tc = calcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);


    ReportData report = ReportData(
        _time,
        g.dotName,
        g.dotPath,
        "yotoBase",
        cacheMisses,
        tries,
        swaps,
        alg_type,
        tc,
        c2n,
        n2c
    );
    return report;
}
