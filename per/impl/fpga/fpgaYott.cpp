#include <common/parameters.h>
#include <common/cache.h>
#include <fpga/fpgaYott.h>


FpgaReportData fpgaYott(FPGAGraph& g)
{
    int nCells = g.nCells;
    int nCellsSqrt = g.nCellsSqrt;
    int nNodes = g.nNodes;

    vector c2n(nCells, -1);
    vector n2c(nNodes, -1);
    vector<vector<int>> distCells = fpgaGetAdjCellsDist(nCellsSqrt);
    vector<int> inOutCells = g.getInOutPos();

    randomVector(inOutCells);

    int cacheMisses = 0;
    int tries = 0;
    int swaps = 0;

    string alg_type;
    vector<pair<int, int>> ed;

    vector<pair<int, int>> convergence;
    ed = g.getEdgesZigzag(convergence);
    unordered_map<string, vector<pair<int, int>>> annotations = g.fpgaGetGraphAnnotations(ed, convergence);

    alg_type = "ZIG_ZAG";

    //int lastIdxIOCellUsed = 0;

    auto start = chrono::high_resolution_clock::now();

    for (auto [a,b] : ed)
    {
        bool placed = false;
#ifdef DEBUG
        fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
        if (n2c[a] == -1)
        {
            const int ioCell = inOutCells.back();
            inOutCells.pop_back();

            if (c2n[ioCell] == -1)
            {
                c2n[ioCell] = a;
                n2c[a] = ioCell;
            }
        }

        //Now, if B is placed, go to next edge

        if (n2c[b] != -1)
        {
            continue;
        }

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const int cellA = n2c[a];
        const int lA = cellA / nCellsSqrt;
        const int cA = cellA % nCellsSqrt;

        int betterCell = -1;
        int betterCellDist = nCells;

        //Then I will look for a cell next to A's cell
        for (const auto& ij : distCells)
        {
            ++tries;
            const int lB = lA + ij[0];
            const int cB = cA + ij[1];
            //find the idx for the target cell
            const int targetCell = lB * nCellsSqrt + cB;
            const int targetCellDist = getManhattanDist(cellA, targetCell, nCellsSqrt);

            // Check if the target cell is nor allowed, go to next
            if (fpgaIsInvalidCell(targetCell, nCellsSqrt))
                continue;

            const bool isTargetCellIO = fpgaIsIOCell(targetCell, nCellsSqrt);
            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            //prevents IO nodes to be not put in IO cells
            //and put a non IO node in an IO cell
            if (isTargetCellIO)
            {
                // 'targetCell' is a IO cell
                if (!IsBIoNode)
                {
                    continue;
                }
            }
            else
            {
                // 'targetCell' is not in possible_positions
                if (IsBIoNode)
                {
                    continue;
                }
            }

            //begin of yott verifications.
            // if yott verifications does not match until targetCellDistance < 4, then
            //the better found cell will be used and if it is not set, yoto will be executed
            //the code needs to be improved, but ir works

            //beginning with an annotation if it exists
            string key = to_string(a) + " " + to_string(b);
            vector<pair<int, int>> annotation = annotations[key];

            //if we have an annotation
            if (!annotation.empty())
            {
                if (targetCellDist < 4)
                {
                    if (c2n[targetCell] != -1)
                        continue;

                    int modDist = targetCellDist;
                    bool found = true;
                    //find the distance of the target cell to the annotated cell and compare if they are equal
                    for (auto& [fst, snd] : annotation)
                    {
                        int annDist;
                        int tAnnDist;

                        if (fst == -1)
                        {
                            annDist = 1;
                            tAnnDist = fpgaMinBorderDist(targetCell, nCellsSqrt);
                        }
                        else
                        {
                            int annCell;
                            annCell = n2c[fst];
                            annDist = snd + 1;
                            tAnnDist = getManhattanDist(targetCell, annCell, nCellsSqrt);
                        }

                        if (tAnnDist != annDist)
                        {
                            found = false;
                            modDist += abs(annDist - tAnnDist);
                        }
                    }
                    if (found)
                    {
                        if (c2n[targetCell] == -1)
                        {
                            c2n[targetCell] = b;
                            n2c[b] = targetCell;
                            ++swaps;
                            break;
                        }
                        continue;
                    }

                    if (modDist < betterCellDist && c2n[targetCell] == -1)
                    {
                        betterCellDist = modDist;
                        betterCell = targetCell;
                    }
                    continue;
                }
                if (betterCell > -1)
                {
                    c2n[betterCell] = b;
                    n2c[b] = betterCell;
                    ++swaps;
                    break;
                }
                if (c2n[targetCell] == -1)
                {
                    c2n[targetCell] = b;
                    n2c[b] = targetCell;
                    ++swaps;
                    break;
                }
            }

            // if the target node has no annotations, then it will put it on the first empty cell
            if (c2n[targetCell] == -1)
            {
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
    auto _time = static_cast<float>(duration.count());

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
