#include <fpga/fpgaYoto.h>
#include <fpga/fpgaPar.h>
#include <common/cache.h>
#include <vector>
#include <map>

using namespace std;


FpgaReportData fpgaYoto(FPGAGraph &g) {
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;

#ifdef CACHE
    auto cacheC2N = Cache();
    auto cacheN2C = Cache();
#endif


    std::vector hist(nCells, 0L);

    vector<long> c2n(nCells, -1);
    vector<long> n2c(nNodes, -1);

    //todo posicionamento Melhorar um dia
    //todo Direcionar após dist 4 a 8 testar o posicionamento de CLBs
    vector<vector<vector<long> > > distCells;
    for (int i = 0; i < N_DIST_VECTORS; i++) {
        distCells.push_back(fpgaGetAdjCellsDist(nCellsSqrt));
    }
    //vector<long> delta = g.gerarDyIntercalado();
    vector<long> inOutCells = g.getInOutPos();

    randomVector(inOutCells);

    long cacheMisses = 0;
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
    ed = g.getEdgesDepthFirstOutFirst();
    alg_type = "DEPTH_FIRST";
#endif

    vector heatEnd(nCells, 0L);
    vector heatBegin(nCells, 0L);
    vector<map<long, long> > histogramFull;
    std::map<long, vector<long> > orDest;

#ifdef MAKE_METRICS

    map<long, long> histogram;
    long maxEd = static_cast<long>(ed.size());
    //(A + (B-1)) / B
    long edOffset = (maxEd + (10 - 1)) / 10;
    long edCounter = 0;
#endif

    //IO placement control
    vector<vector<long> > searchSequence = g.generateOffsets();
    std::vector<BorderInfo> borderSequence;
    long sequenceCounter = 0;
    long dCounter = 0;
    bool setBorder = false;
    bool blockNegative = false;
    bool blockPositive = false;
    int tickTack = 0;

#ifdef PRINT
    fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

    //time counting
    auto start = chrono::high_resolution_clock::now();
    long unicTry;

    long distVectorCounter = 0;

    for (auto [a,b]: ed) {
        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks

        distVectorCounter++;
        if (distVectorCounter >= N_DIST_VECTORS) distVectorCounter = 0;

#ifdef CACHE
        cacheMisses += cacheN2C.readCache(a, n2c);
#endif
        const long cellA = n2c[a];


#ifdef MAKE_METRICS
        edCounter++;
        const long _off = edCounter / edOffset;
        if (_off > histogramFull.size()) {
            histogramFull.push_back(histogram);
        }

        unicTry = 0;
#endif


        long targetNode = -1;

        if (a == -1) {
            targetNode = b;
        } else if (cellA == -1) {
            cout << "Error while placing A node";
            exit(1);
        }

        if (targetNode != -1) {
            bool found = false;
            while (!found) {
                ioTries++;
                unicTry++;
                const long ioCell = inOutCells.back();
                inOutCells.pop_back();
#ifdef CACHE
                cacheMisses += cacheC2N.readCache(ioCell, c2n);
#endif
                c2n[ioCell] = targetNode;
                n2c[targetNode] = ioCell;
                found = true;
                swaps++;
#ifdef MAKE_METRICS

                if (histogram.find(unicTry) != histogram.end()) {
                    histogram[unicTry]++;
                } else {
                    histogram[unicTry] = 1;
                }
                heatEnd[ioCell] = unicTry;
#endif
            }
            continue;
        }

        //Now, if B is placed, go to next edge
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(b, n2c);
#endif
        if (n2c[b] != -1)
            continue;

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const long lA = cellA / nCellsSqrt;
        const long cA = cellA % nCellsSqrt;


        //Then I will look for a cell next to A's cell
        for (const auto &ij: distCells[distVectorCounter]) {
            unicTry++;

            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            if (IsBIoNode) {
                ioTries++;
            } else {
                clbTries++;
            }

            long targetCell;
            long lB;
            long cB;
            long dist;

            if (IsBIoNode) {
                if (!setBorder) {
                    borderSequence = g.getBordersSequence(lA, cA);
                    sequenceCounter = 0;
                    dCounter = 0;
                    blockNegative = false;
                    blockPositive = false;
                    setBorder = true;
                    tickTack = 0;
                }

                long d;
                if (blockNegative) {
                    d = searchSequence[0][dCounter];
                    dCounter++;
                } else if (blockPositive) {
                    d = searchSequence[1][dCounter];
                    dCounter++;
                } else {
                    d = searchSequence[tickTack][dCounter];
                    if (tickTack == 0) {
                        tickTack = 1;
                    } else {
                        tickTack = 0;
                        dCounter++;
                    }
                }
                switch (borderSequence[sequenceCounter].direction) {
                    case 0: // top
                        lB = 0;
                        cB = borderSequence[sequenceCounter].coord.first + d;
                        break;
                    case 1: // bottom
                        lB = nCellsSqrt - 1;
                        cB = borderSequence[sequenceCounter].coord.first + d;
                        break;
                    case 2: // left
                        lB = borderSequence[sequenceCounter].coord.second + d;
                        cB = 0;
                        break;
                    case 3: // right
                        lB = borderSequence[sequenceCounter].coord.second + d;
                        cB = nCellsSqrt - 1;
                        break;
                }

                targetCell = lB * nCellsSqrt + cB;

                if (lB < 0 || cB < 0) {
                    blockNegative = true;
                } else if (lB >= nCellsSqrt || cB >= nCellsSqrt)
                    blockPositive = true;
                if (dCounter == nCellsSqrt || (blockNegative && blockPositive)) {
                    dCounter = 0;
                    sequenceCounter += 1;
                    blockNegative = false;
                    blockPositive = false;
                    tickTack = 0;
                }
            } else {
                lB = lA + ij[0];
                cB = cA + ij[1];

                //find the idx for the target cell
                targetCell = lB * nCellsSqrt + cB;

                const bool isTargetCellIO = fpgaIsIOCell(lB, cB, nCellsSqrt);

                //prevents put a non IO noce in an IO cell
                if (isTargetCellIO)
                    continue;
            }
            const bool isValidCell = fpgaIsInvalidCell(lB, cB, nCellsSqrt);
            // Check if the target cell is nor allowed, go to next
            if (isValidCell)
                continue;
            dist = getManhattanDist(cellA, targetCell, nCellsSqrt);

            // Place the node if `placement[targetCell]` is unoccupied
#ifdef CACHE
            cacheMisses += cacheC2N.readCache(targetCell, c2n);
#endif
            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                setBorder = false;
#ifdef MAKE_METRICS
                if (histogram.find(unicTry) != histogram.end()) {
                    histogram[unicTry]++;
                } else {
                    histogram[unicTry] = 1;
                }
                heatEnd[targetCell] = unicTry;
                heatBegin[cellA]++;
                if (orDest.find(cellA) == orDest.end()) {
                    orDest[cellA] = vector<long>();
                }
                orDest[cellA].push_back(targetCell);
#endif
                long _max = 2 * dist * (dist + 1);
                if ((unicTry > _max) && !IsBIoNode) {
                    int asd = 1;
#ifdef PRINT
                    fpgaSavePlacedDot(n2c, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
                    cout << "Error while placing: dist=" << dist << " max tries:" << _max;
                    cout << ". Total tries" << unicTry;
                    //exit(1);
                }
                break;
            }
        }
    }

#ifdef MAKE_METRICS
    histogramFull.push_back(histogram);
#endif

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

    //fixme
    const long tlpc = 0; //fpgaCalcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
    //#endif

    const long tries = (clbTries + ioTries);
    long cachePenalties = CACHE_W_PARAMETER * CACHE_W_COST * cacheMisses;
    const long triesP = tries + cachePenalties;
    const long nIOs = static_cast<long>(g.outputNodes.size() + g.inputNodes.size());
    if (swaps != nNodes) {
        cout << "Erro ao processar o arquivo " << g.dotName << "Nem todo os nós foram posicionados" << endl;
    }

    //FIXME reports
    auto report = FpgaReportData(
        _time,
        g.dotName,
        g.dotPath,
        "yoto",
        nCellsSqrt,
        nNodes,
        nIOs,
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
        histogramFull,
        heatEnd,
        heatBegin,
        orDest
    );
    return report;
}
