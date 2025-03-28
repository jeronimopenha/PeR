#include "yotoBase.h"


ReportData yotoBase(Graph& g)
{
    int nCells = g.nCells;
    int nCellsSqrt = g.nCellsSqrt;
    int nNodes = g.nNodes;

    vector<int> c2n(nCells, -1);
    vector<int> n2c(nNodes, -1);
    vector<vector<int>> distCells = getAdjCellsDist(nCellsSqrt);
    vector<int> inOutCells = g.getInOutPos();
#ifdef YOTO_BASE_ZZ_CACHE
    Cache cacheC2N = Cache(CACHE_LINES_EXP, CACHE_COLUMNS_EXP);
    Cache cacheN2C = Cache(CACHE_LINES_EXP, CACHE_COLUMNS_EXP);
#endif
    randomVector(inOutCells);

    int cacheMisses = 0;
    int tries = 0;
    int swaps = 0;

    string alg_type;
    vector<pair<int, int>> ed;
#if defined(YOTO_BASE_ZZ) || defined(YOTO_BASE_ZZ_CACHE)
    vector<pair<int, int> > convergence;
    ed = g.getEdgesZigzag(convergence);
    alg_type = "ZIG_ZAG";
#elifdef YOTO_BASE_DF_P
    ed = getEdgesDepthFirstPriority();
    alg_type = "DEPTH_FIRST_PRIORITY";
#elifdef YOTO_BASE_DF
    ed = g.getEdgesDepthFirst();
    alg_type = "DEPTH_FIRST";
#endif

    //saveToDot(ed, "/home/jeronimo/test.dot");
    int lastIdxIOCellUsed = 0;

#if defined(YOTO_BASE_DF) || defined(YOTO_BASE_DF_P)
    //for Deptfh First Search with or without priority
    //I need to place every input at the beginning of execution
    for (int n: g.inputNodes) {
        for (int i = lastIdxIOCellUsed + 1; i < inOutCells.size(); i++) {
            int ioCell = inOutCells[i];
            if (c2n[ioCell] == -1) {
                c2n[ioCell] = n;
                n2c[n] = ioCell;
                lastIdxIOCellUsed = i;
                break;
            }
        }
    }
#endif
    auto start = chrono::high_resolution_clock::now();

    for (auto [a,b] : ed)
    {
        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
#ifdef YOTO_BASE_ZZ_CACHE
        cacheMisses += cacheN2C.checkCache(a, n2c);
#endif

        if (n2c[a] == -1)
        {
            for (int i = lastIdxIOCellUsed + 1; i < inOutCells.size(); i++)
            {
                int ioCell = inOutCells[i];
#ifdef YOTO_BASE_ZZ_CACHE
                cacheMisses += cacheC2N.checkCache(ioCell, c2n);
#endif
                if (c2n[ioCell] == -1)
                {
                    c2n[ioCell] = a;
                    n2c[a] = ioCell;
                    lastIdxIOCellUsed = i;
                    break;
                }
            }
        }

        //Now, if B is placed, go to next edge
#ifdef YOTO_BASE_ZZ_CACHE
        cacheMisses += cacheN2C.checkCache(b, n2c);
#endif
        if (n2c[b] != -1)
        {
            continue;
        }

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const int lA = n2c[a] / nCellsSqrt;
        const int cA = n2c[a] % nCellsSqrt;

        //bool placed = false;
        //Then I will look for a cell next to A's cell
        for (const auto& ij : distCells)
        {
            ++tries;
            const int lB = lA + ij[0];
            const int cB = cA + ij[1];

            //find the idx for the target cell
            int targetCell = lB * nCellsSqrt + cB;

            // Check if the target cell is nor allowed, go to next
            if (isInvalidCell(targetCell, nCellsSqrt))
                continue;


            const bool isTargetCellIO = isIOCell(targetCell, nCellsSqrt);
            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            //prevents IO nodes to be not put in IO cells
            //and put a non IO noce in an IO cell
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

            // Place the node if `placement[targetCell]` is unoccupied
#ifdef YOTO_BASE_ZZ_CACHE
            cacheMisses += cacheC2N.checkCache(targetCell, c2n);
#endif
            if (c2n[targetCell] == -1)
            {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                //placed = true;
                break;
            }
        }
    }

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
