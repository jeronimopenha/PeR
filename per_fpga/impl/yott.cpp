#include "yott.h"
#include <cache.h>

ReportData yott(Graph &g) {
    int nCells = g.nCells;
    int nCellsSqrt = g.nCellsSqrt;
    int nNodes = g.nNodes;

    vector<int> c2n(nCells, -1);
    vector<int> n2c(nNodes, -1);
    vector<vector<int> > distCells = getAdjCellsDist(nCellsSqrt);
    vector<int> inOutCells = g.getInOutPos();
#ifdef CACHE
    Cache cacheC2N = Cache();
    Cache cacheN2C = Cache();
#endif
    randomVector(inOutCells);

    int cacheMisses = 0;
    int tries = 0;
    int swaps = 0;

    string alg_type;
    vector<pair<int, int> > ed;

    vector<pair<int, int> > convergence;
    ed = g.getEdgesZigzag(convergence);
    unordered_map<string, vector<pair<int, int> > > annotations = g.getGraphAnnotations(ed, convergence);
#ifdef YOTT_IO
    g.getIOAnnotations(annotations, ed);
#endif
    alg_type = "ZIG_ZAG";


    //saveToDot(ed, "/home/jeronimo/test.dot");
    int lastIdxIOCellUsed = 0;
#ifdef YOTT_IO
    //for Deptfh First Search with or without priority
    //I need to place every input at the beginning of execution
    for (int n : g.inputNodes)
    {
        int ioCell = inOutCells[lastIdxIOCellUsed];
        if (c2n[ioCell] == -1)
        {
            c2n[ioCell] = n;
            n2c[n] = ioCell;
            lastIdxIOCellUsed++;
        }
    }
#endif

    auto start = chrono::high_resolution_clock::now();

    for (auto [a,b]: ed) {
#ifdef DEBUG
        savePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(a, n2c);
#endif

        if (n2c[a] == -1) {
            int ioCell = inOutCells[lastIdxIOCellUsed];
#ifdef CACHE
                cacheMisses += cacheC2N.readCache(ioCell, c2n);
#endif
            if (c2n[ioCell] == -1) {
                c2n[ioCell] = a;
                n2c[a] = ioCell;
                lastIdxIOCellUsed++;
            }
        }

        //Now, if B is placed, go to next edge
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(b, n2c);
#endif
        if (n2c[b] != -1) {
            continue;
        }

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const int cellA = n2c[a];
        const int lA = cellA / nCellsSqrt;
        const int cA = cellA % nCellsSqrt;

        int betterCell = -1;
        int betterCellDist = nCells;

        //bool placed = false;
        //Then I will look for a cell next to A's cell

        for (const auto &ij: distCells) {
            ++tries;
            const int lB = lA + ij[0];
            const int cB = cA + ij[1];
            //find the idx for the target cell
            const int targetCell = lB * nCellsSqrt + cB;
            const int targetCellDist = getManhattanDist(cellA, targetCell, nCellsSqrt);

            // Check if the target cell is nor allowed, go to next
            if (isInvalidCell(targetCell, nCellsSqrt))
                continue;

            const bool isTargetCellIO = isIOCell(targetCell, nCellsSqrt);
            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            //prevents IO nodes to be not put in IO cells
            //and put a non IO node in an IO cell
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

            //begin of yott verifications.
            // if yott verifications does not match until targetCellDistance < 4, then
            //the better found cell will be used and if it is not set, yoto will be executed
            //the code needs to be improved, but ir works

            //beginning with an annotation if it exists
            string key = to_string(a) + " " + to_string(b);
            vector<pair<int, int> > annotation = annotations[key];

            //if we have an annotation
            if (!annotation.empty()) {
                if (targetCellDist < 4) {
                    if (c2n[targetCell] != -1)
                        continue;

                    int modDist = targetCellDist;
                    bool found = true;
                    //find the distance of the target cell to the annotated cell and compare if they are equal
                    for (auto [fst, snd]: annotation) {
                        int annCell;
                        int annDist;
                        int tAnnDist;

                        if (fst == -1) {
                            annDist = 1;
                            tAnnDist = minBorderDist(targetCell, nCellsSqrt);
                        } else {
                            annCell = n2c[fst];
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
#ifdef CACHE
            cacheMisses += cacheC2N.readCache(targetCell, c2n);
#endif
            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                break;
            }
        }
    }
#ifdef DEBUG
    savePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    float _time = duration.count();

    // commented to take the cost of the longest path
#ifdef TOTAL_COST
    int tc = calcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
#elifdef LP_COST
    int tc = calcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
#endif

    ReportData report = ReportData(
        _time,
        g.dotName,
        g.dotPath,
        "yott",
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
