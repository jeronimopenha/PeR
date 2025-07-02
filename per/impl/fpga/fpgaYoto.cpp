#include <fpga/fpgaYoto.h>
#include <fpga/fpgaPar.h>
#include <common/cache.h>
#include <vector>

using namespace std;


FpgaReportData fpgaYoto(FPGAGraph &g) {
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;

    std::vector hist(nCells, 0L);

    vector<long> c2n(nCells, -1);
    vector<long> n2c(nNodes, -1);
    //todo começar com saídas
    //todo posicionametno de IOs
    //todo Direcionar
    vector<vector<long> > distCells = fpgaGetAdjCellsDist(nCellsSqrt);
    vector<long> inOutCells = g.getInOutPos();
#ifdef CACHE
    auto cacheC2N = Cache();
    auto cacheN2C = Cache();
#endif
    randomVector(inOutCells);

    long cacheMisses = 0;
    //long tries = 0;
    long ioTries = 0;
    long clbTries = 0;
    long swaps = 0;

    string alg_type;
    vector<pair<long, long> > ed;
#if defined(FPGA_YOTO_ZZ)
    vector<pair<long, long>> convergence;
    ed = g.getEdgesZigzag(convergence);
    alg_type = "ZIG_ZAG";
#elifdef FPGA_YOTO_DF_PRIO
    ed = g.getEdgesDepthFirstPriority();
    alg_type = "DEPTH_FIRST_PRIORITY";
#elifdef FPGA_YOTO_DF
    ed = g.getEdgesDepthFirst();
    alg_type = "DEPTH_FIRST";
#endif


#ifndef FPGA_YOTO_ZZ
    //for Deptfh First Search with or without priority
    //I need to place every input at the beginning of execution
    for (auto n: g.inputNodes) {
        const long ioCell = inOutCells.back();
        inOutCells.pop_back();
        if (c2n[ioCell] == -1) {
            c2n[ioCell] = n;
            n2c[n] = ioCell;
        }
    }
#endif

    //time counting
    auto start = chrono::high_resolution_clock::now();

    for (auto [a,b]: ed) {
        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(a, n2c);
#endif

        if (n2c[a] == -1) {
            bool found = false;
            while (!found) {
                const long ioCell = inOutCells.back();
                inOutCells.pop_back();
#ifdef CACHE
                cacheMisses += cacheC2N.readCache(ioCell, c2n);
#endif
                if (c2n[ioCell] == -1) {
                    c2n[ioCell] = a;
                    n2c[a] = ioCell;
                    found = true;
                }
            }
        }

        //Now, if B is placed, go to next edge
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(b, n2c);
#endif
        if (n2c[b] != -1)
            continue;

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(a, n2c);
#endif
        const long lA = n2c[a] / nCellsSqrt;
        const long cA = n2c[a] % nCellsSqrt;

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
            long targetCell = lB * nCellsSqrt + cB;

            // Check if the target cell is nor allowed, go to next
            if (fpgaIsInvalidCell(targetCell, nCellsSqrt))
                continue;


            const bool isTargetCellIO = fpgaIsIOCell(targetCell, nCellsSqrt);


            //prevents IO nodes to be not put in IO cells
            //and put a non IO noce in an IO cell
            if (isTargetCellIO) {
                // 'targetCell' is a IO cell
                if (!IsBIoNode)
                    continue;
            } else {
                // 'targetCell' is not in possible_positions
                if (IsBIoNode)
                    continue;
            }

            // Place the node if `placement[targetCell]` is unoccupied
#ifdef CACHE
            cacheMisses += cacheC2N.readCache(targetCell, c2n);
#endif
            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                //placed = true;
                break;
            }
        }
    }

#ifdef PRINT
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
    const long tlpc = fpgaCalcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
    //#endif

    const long tries = (clbTries + ioTries);
    long cachePenalties = CACHE_W_PARAMETER * CACHE_W_COST * cacheMisses;
    const long triesP = tries + cachePenalties;

    //FIXME reports
    auto report = FpgaReportData(
        _time,
        g.dotName,
        g.dotPath,
        "yoto",
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
        n2c,
        std::vector<std::vector<long> >()
    );
    return report;
}
