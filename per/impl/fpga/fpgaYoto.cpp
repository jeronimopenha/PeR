#include <fpga/fpgaYoto.h>
#include <fpga/fpgaPar.h>
#include <common/cache.h>
#include <vector>
#include <map>

using namespace std;

//todo posicionamento Melhorar um dia
//ideia colocar blocks de neg e pos para col e cell pra tudo
//pra isso vetores de offset serão gerados por delta col e delta line
//todo Direcionar após dist 4 a 8 testar o posicionamento de CLBs
//ainda falta implementar
FpgaReportData fpgaYoto(FPGAGraph &g) {
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;

    vector<vector<long> > c2n(nCells, vector<long>());
    vector<pair<long, long> > n2c(nNodes, {-1, -1});

    vector<vector<vector<long> > > distCells;
    vector<pair<long, long> > ed;
    vector<long> inOutCells = g.getInOutPos();

    long ioTries = 0;
    long clbTries = 0;
    long swaps = 0;

    string alg_type;

#ifdef CACHE
    auto cacheC2N = Cache();
    auto cacheN2C = Cache();
#endif
    //fixme cache variable
    long cacheMisses = 0;

    //fixme metrics variables
    std::vector hist(nCells, 0L);
    vector heatEnd(nCells, 0L);
    vector heatBegin(nCells, 0L);
    vector<map<long, long> > histogramFull;
    std::map<long, vector<long> > originDestin;


    //fill the distCells vector
    for (int i = 0; i < N_DIST_VECTORS; i++) {
        distCells.push_back(fpgaGetAdjCellsDist(nCellsSqrt));
    }

    //shuffle the possible IO cells
    randomVector(inOutCells);

    //choosing the edges generator by algorithm type
    //todo I->O  and O->I (the last one is implemented)
    //todo with critical path priority or not
#if defined(FPGA_YOTO_ZZ)
    vector<pair<long, long> > convergence;
    ed = g.getEdgesZigzag(convergence);
    alg_type = "ZIG_ZAG";
#elifdef FPGA_YOTO_DF_PRIO
    ed = g.getEdgesDepthFirstPriority();
    alg_type = "DEPTH_FIRST_PRIORITY";
#elifdef FPGA_YOTO_DF
    ed = g.getEdgesDepthFirstCritical();
    alg_type = "DEPTH_FIRST";
#endif


    //filling the metrics variables
#ifdef MAKE_METRICS
    map<long, long> histogram;
    long maxEd = static_cast<long>(ed.size());
    long edOffset = (maxEd + (10 - 1)) / 10; //(A + (B-1)) / B
    long edCounter = 0;
#endif

    //IO placement control
    vector<vector<long> > ioSearchSequence = g.generateIoOffsets();
    std::vector<BorderInfo> ioBorderSequence;
    long ioSequenceCounter = 0;
    long ioDistCounter = 0;
    bool ioSetBorder = false;
    bool ioBlockBorderNegative = false;
    bool ioBlockBorderPositive = false;
    int ioBorderTickTack = 0;

#ifdef PRINT
    fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

    //time counting
    auto start = chrono::high_resolution_clock::now();

    long distVectorCounter = 0;

    for (auto [a,b]: ed) {
#ifdef MAKE_METRICS
        edCounter++;
        const long _off = edCounter / edOffset;
        if (_off > static_cast<long>(histogramFull.size())) {
            histogramFull.push_back(histogram);
        }

        long unicTry = 0;
#endif

        long targetNode = -1;
        bool nextEdge = false;
        if (a == -1) {
            targetNode = b;
            nextEdge = true;
        } else if (n2c[a].first == -1) {
            targetNode = a;
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
                if (static_cast<long>(c2n[ioCell].size()) < IO_NUMBER) {
                    c2n[ioCell].push_back(targetNode);
                    n2c[targetNode].first = ioCell;
                    n2c[targetNode].second = static_cast<long>(c2n[ioCell].size()) - 1;
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
            }
#ifdef PRINT
            fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
            if (nextEdge)
                continue;
            unicTry = 0;
        }

        distVectorCounter = (distVectorCounter < N_DIST_VECTORS - 1) ? distVectorCounter + 1 : 0;

        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(a, n2c);
#endif
        const long cellA = n2c[a].first;

        //Now, if B is placed, go to next edge
#ifdef CACHE
        cacheMisses += cacheN2C.readCache(b, n2c);
#endif
        if (n2c[b].first != -1)
            continue;

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const long lA = cellA / nCellsSqrt;
        const long cA = cellA % nCellsSqrt;

        bool isTargetCellIO = true;


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
                if (!ioSetBorder) {
                    ioBorderSequence = g.getIoBordersSequence(lA, cA);
                    ioSequenceCounter = 0;
                    ioDistCounter = 0;
                    ioBlockBorderNegative = false;
                    ioBlockBorderPositive = false;
                    ioSetBorder = true;
                    ioBorderTickTack = 0;
                }

                long ioD;
                if (ioBlockBorderNegative) {
                    ioD = ioSearchSequence[0][ioDistCounter];
                    ioDistCounter++;
                } else if (ioBlockBorderPositive) {
                    ioD = ioSearchSequence[1][ioDistCounter];
                    ioDistCounter++;
                } else {
                    ioD = ioSearchSequence[ioBorderTickTack][ioDistCounter];
                    if (ioBorderTickTack == 0) {
                        ioBorderTickTack = 1;
                    } else {
                        ioBorderTickTack = 0;
                        ioDistCounter++;
                    }
                }
                switch (ioBorderSequence[ioSequenceCounter].direction) {
                    case 0: // top
                        lB = 0;
                        cB = ioBorderSequence[ioSequenceCounter].coord.first + ioD;
                        break;
                    case 1: // bottom
                        lB = nCellsSqrt - 1;
                        cB = ioBorderSequence[ioSequenceCounter].coord.first + ioD;
                        break;
                    case 2: // left
                        lB = ioBorderSequence[ioSequenceCounter].coord.second + ioD;
                        cB = 0;
                        break;
                    case 3: // right
                        lB = ioBorderSequence[ioSequenceCounter].coord.second + ioD;
                        cB = nCellsSqrt - 1;
                        break;
                }

                targetCell = lB * nCellsSqrt + cB;

                if (lB < 0 || cB < 0) {
                    ioBlockBorderNegative = true;
                } else if (lB >= nCellsSqrt || cB >= nCellsSqrt)
                    ioBlockBorderPositive = true;
                if (ioDistCounter == nCellsSqrt || (ioBlockBorderNegative && ioBlockBorderPositive)) {
                    ioDistCounter = 0;
                    ioSequenceCounter += 1;
                    ioBlockBorderNegative = false;
                    ioBlockBorderPositive = false;
                    ioBorderTickTack = 0;
                }
            } else {
                lB = lA + ij[0];
                cB = cA + ij[1];

                isTargetCellIO = fpgaIsIOCell(lB, cB, nCellsSqrt);
                //prevents put a non IO node in an IO cell
                if (isTargetCellIO)
                    continue;

                //find the idx for the target cell
                targetCell = lB * nCellsSqrt + cB;
            }
            const bool isInvalidCell = fpgaIsInvalidCell(lB, cB, nCellsSqrt);
            // Check if the target cell is nor allowed, go to next
            if (isInvalidCell)
                continue;
            dist = getManhattanDist(cellA, targetCell, nCellsSqrt);

            const bool ioPlaceFlag = (isTargetCellIO && static_cast<long>(c2n[targetCell].size()) < IO_NUMBER);
            const bool clbPlaceFlag = (!isTargetCellIO && c2n[targetCell].empty());
            const bool placeFlag = ioPlaceFlag || clbPlaceFlag;


            // Place the node if `placement[targetCell]` is unoccupied
#ifdef CACHE
            cacheMisses += cacheC2N.readCache(targetCell, c2n);
#endif
            if (placeFlag) {
                c2n[targetCell].push_back(b);
                /*if (static_cast<long>(c2n[targetCell].size()) > 3) {
                    int asd = 1;
                }*/
                n2c[b].first = targetCell;
                n2c[b].second = static_cast<long>(c2n[targetCell].size()) - 1;
                ++swaps;
                ioSetBorder = false;
#ifdef MAKE_METRICS
                if (histogram.find(unicTry) != histogram.end()) {
                    histogram[unicTry]++;
                } else {
                    histogram[unicTry] = 1;
                }
                heatEnd[targetCell] = unicTry;
                heatBegin[cellA]++;
                if (originDestin.find(cellA) == originDestin.end()) {
                    originDestin[cellA] = vector<long>();
                }
                originDestin[cellA].push_back(targetCell);
#endif
#ifdef PRINT
                fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
                long _max = 2 * dist * (dist + 1);
                if ((unicTry > _max) && !IsBIoNode) {
                    int asd = 1;

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
    fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    auto _time = duration.count();

    //long tc = 0;
    // commented to take the cost of the longest path
    //#ifdef FPGA_TOTAL_COST
    //fixme
    const long tc = -1; //fpgaCalcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
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
        0,
        swaps,
        alg_type,
        tc,
        tlpc,
        c2n,
        n2c,
        histogramFull,
        heatEnd,
        heatBegin,
        originDestin
    );
    return report;
}
