#include <fpga/fpgaPar.h>
#include <common/cache.h>
#include <fpga/fpgaYott.h>

using namespace std;

FpgaReportData fpgaYott(FPGAGraph &g) {
    /*
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;

    vector<long> c2n(nCells, -1);
    vector<long> n2c(nNodes, -1);
    const vector<vector<long> > distCells = fpgaGetAdjCellsDist(nCellsSqrt);
    vector<long> inOutCells = g.getInOutPos();

    randomVector(inOutCells);

    long cacheMisses = 0;
    //long tries = 0;
    long ioTries = 0;
    long clbTries = 0;
    long swaps = 0;

    string alg_type = "ZIG_ZAG";

    vector<pair<long, long> > convergence;
    vector<pair<long, long> > ed = g.getEdgesZigzag(convergence);
    unordered_map<string, vector<pair<long, long> > > annotations = g.fpgaGetGraphAnnotations(ed, convergence);

    //int lastIdxIOCellUsed = 0;

    auto start = chrono::high_resolution_clock::now();


    for (auto [a,b]: ed) {
#ifdef DEBUG
        //fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
        if (n2c[a] == -1) {
            bool found = false;
            while (!found) {
                const long ioCell = inOutCells.back();
                inOutCells.pop_back();

                if (c2n[ioCell] == -1) {
                    c2n[ioCell] = a;
                    n2c[a] = ioCell;
                    found = true;
                }
            }
        }

        //Now, if B is placed, go to next edge

        if (n2c[b] != -1)
            continue;


        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const long cellA = n2c[a];
        const long lA = cellA / nCellsSqrt;
        const long cA = cellA % nCellsSqrt;

        long betterCell = -1;
        long betterCellDist = nCells;

        //Then I will look for a cell next to A's cell
        for (const auto &ij: distCells) {
            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            if (IsBIoNode) {
                ioTries++;
            } else {
                clbTries++;
            }
            //++tries;

            const long lB = lA + ij[0];
            const long cB = cA + ij[1];
            //find the idx for the target cell
            const long targetCell = lB * nCellsSqrt + cB;
            const long targetCellDist = getManhattanDist(cellA, targetCell, nCellsSqrt);

            // Check if the target cell is nor allowed, go to next
            if (fpgaIsInvalidCell(lB,cB, nCellsSqrt))
                continue;

            const bool isTargetCellIO = fpgaIsIOCell(lB,cB, nCellsSqrt);
            //const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            //prevents IO nodes to be not put in IO cells
            //and put a non IO node in an IO cell
            if (isTargetCellIO) {
                // 'targetCell' is a IO cell
                if (!IsBIoNode)
                    continue;
            } else {
                // 'targetCell' is not in possible_positions
                if (IsBIoNode)
                    continue;
            }

            //begin of yott verifications.
            // if yott verifications does not match until targetCellDistance < 4, then
            //the better found cell will be used and if it is not set, yoto will be executed
            //the code needs to be improved, but ir works

            //beginning with an annotation if it exists
            string key = to_string(a) + " " + to_string(b);
            vector<pair<long, long> > annotation = annotations[key];

            //if we have an annotation
            if (!annotation.empty()) {
                if (targetCellDist < 4) {
                    if (c2n[targetCell] != -1)
                        continue;

                    long modDist = targetCellDist;
                    bool found = true;
                    //find the distance of the target cell to the annotated cell and compare if they are equal
                    for (auto &[fst, snd]: annotation) {
                        long annDist;
                        long tAnnDist;

                        if (fst == -1) {
                            annDist = 1;
                            tAnnDist = fpgaMinBorderDist(targetCell, nCellsSqrt);
                        } else {
                            const long annCell = n2c[fst];
                            annDist = snd + 1;
                            tAnnDist = getManhattanDist(targetCell, annCell, nCellsSqrt);
                        }

                        if (tAnnDist != annDist) {
                            found = false;
                            modDist += abs(annDist - tAnnDist);
                        }
                    }
                    if (found) {
                        if (c2n[targetCell] == -1) {
                            c2n[targetCell] = b;
                            n2c[b] = targetCell;
                            ++swaps;
                            break;
                        }
                        continue;
                    }

                    if (modDist < betterCellDist && c2n[targetCell] == -1) {
                        betterCellDist = modDist;
                        betterCell = targetCell;
                    }
                    continue;
                }
                if (betterCell > -1) {
                    c2n[betterCell] = b;
                    n2c[b] = betterCell;
                    ++swaps;
                    break;
                }
                if (c2n[targetCell] == -1) {
                    c2n[targetCell] = b;
                    n2c[b] = targetCell;
                    ++swaps;
                    break;
                }
            }

            // if the target node has no annotations, then it will put it on the first empty cell
            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                break;
            }
        }
    }
#ifdef DEBUG
    fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    auto _time = duration.count();

    //long tc = 0;
    // commented to take the cost of the longest path
    //#ifdef FPGA_TOTAL_COST
    const long tc = fpgaCalcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
    //#elifdef FPGA_LONG_PATH_COST
    //fixme
    //const long tlpc = fpgaCalcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
    //#endif

    const long tries = (clbTries + ioTries);
    long cachePenalties = CACHE_W_PARAMETER * CACHE_W_COST * cacheMisses;
    const long triesP = tries + cachePenalties;
    */

    /*auto report = FpgaReportData(
        _time,
        g.dotName,
        g.dotPath,
        "yott",
        cacheMisses,
        CACHE_W_PARAMETER,
        CACHE_W_COST,
        cachePenalties,
        clbTries,
        ioTries,
        tries,
        triesP,
        swaps,
        alg_type,
        tc,
        tlpc,
        c2n,
        n2c
    );*/

    return FpgaReportData();
}
