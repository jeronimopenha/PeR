#include <fpga/fpgaYoto.h>
#include <fpga/fpgaPar.h>
#include <common/cache.h>
#include <vector>
#include <map>
#include <unordered_set>

using namespace std;

//todo posicionamento Melhorar um dia
//ideia colocar blocks de neg e pos para col e cell pra tudo
FpgaReportData fpgaYoto(FPGAGraph &g) {
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;
    const long nEdges = g.nEdges;

    vector<vector<long> > c2n(nCells, vector<long>());
    vector<pair<long, long> > n2c(nNodes, {-1, -1});

    vector<vector<vector<long> > > distCells;
    vector<pair<long, long> > ed;
    vector<long> inOutCells = g.getInOutPos();

    long ioTries = 0;
    long clbTries = 0;
    long swaps = 0;

    string alg_type;

#ifdef SCAN_STRATEGY
    vector<vector<long> > scannedCells(SCAN_QUADRANTS);
#endif


    //fixme metrics variables
#ifdef USE_CACHE
    long cacheMisses = 0;
    auto cacheC2N = Cache();
    auto cacheN2C = Cache();
#else
    constexpr long cacheMisses = 0;
#endif
    //fixme cache variable

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
#endif
    long maxEd = static_cast<long>(ed.size());
    long edCounter = -1;
    //fixme Transform 10 in totalSnapshots PARAMETER!!!
    constexpr int totalSnapshots = 10;
    const long snapshotInterval = maxEd / totalSnapshots;
    long nextSnapshotAt = snapshotInterval;
    int snapId = 0;

    //IO placement control
    vector<vector<long> > ioSearchSequence = g.generateIoOffsets();
    std::vector<BorderInfo> ioBorderSequence;
    long ioSequenceCounter = 0;
    long ioDistCounter = 0;
    bool ioSetBorder = false;
    bool ioBlockBorderNegative = false;
    bool ioBlockBorderPositive = false;
    int ioBorderTickTack = 0;

#ifdef PRINT_DOT
    fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif

    //time counting
    auto start = chrono::high_resolution_clock::now();

    long distVectorCounter = 0;
    long distSlackCost = 0;

    //YOTO - Begin ****************************8

#ifdef SCAN_STRATEGY
    bool scanned = false;
#endif

#ifdef LIMIT_STRATEGY
    //fixme transform 16 in parameter
    vector<long> limitQuadrants(16, 0l);

    for (long l = 1; l < nCellsSqrt - 1; l++) {
        for (long c = 1; c < nCellsSqrt - 1; c++) {
            const long quadrant = getQuadrant(l, c, nCells, nCellsSqrt); // quad_x * 4 + quad_y;
            limitQuadrants[quadrant]++;
        }
    }
#endif

    for (auto [a,b]: ed) {
        edCounter++;
        const bool snapTaken = edCounter + 1 == nextSnapshotAt;

        if (snapTaken) {
#ifdef MAKE_METRICS
            histogramFull.push_back(histogram);
#endif
            nextSnapshotAt += snapshotInterval;
            snapId++;
        }

#ifdef DEBUG
        bool placed = false;
#endif

        long unicTry = 0;

#ifdef PRINT_IMG
        if (snapTaken)
            writeMap(c2n, {-1, -1}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");
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
#ifdef USE_CACHE
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
#ifdef PRINT_DOT
            fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
#ifdef PRINT_IMG
            if (snapTaken)
                writeMap(c2n, {n2c[a].first, n2c[b].first}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");
#endif
            if (nextEdge)
                continue;
            unicTry = 0;
        }

        distVectorCounter = (distVectorCounter < N_DIST_VECTORS - 1) ? distVectorCounter + 1 : 0;

        //Verify if A is placed
        //if it is not placed, then place in a random inout cell.
        //the variable lastIdxIOCellUsed is for optimize future looks
#ifdef USE_CACHE
        cacheMisses += cacheN2C.readCache(a, n2c);
#endif
        const long cellA = n2c[a].first;

        //Now, if B is placed, go to next edge
#ifdef USE_CACHE
        cacheMisses += cacheN2C.readCache(b, n2c);
#endif
        if (n2c[b].first != -1)
            continue;

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const long lA = cellA / nCellsSqrt;
        const long cA = cellA % nCellsSqrt;

        bool isTargetCellIO = true;

#ifdef LIMIT_STRATEGY
        long limitAcc = 1;
        bool abortLimitStrategy = false;
#endif

#ifdef SCAN_STRATEGY
        long quadCounter = getQuadrant(lA, cA, nCells, nCellsSqrt);
#endif

        //fixme This part is only for YOTO spiral strategy. This should be on that part only
        //Then I will look for a cell next to A's cell
        for (const auto &ij: distCells[distVectorCounter]) {
            long lB = lA + ij[0];
            long cB = cA + ij[1];

            long targetCell;

#ifdef LIMIT_STRATEGY
            bool limitStrategyTrigger = false;
            targetCell = lB * nCellsSqrt + cB;
            const long lmsDist = getManhattanDist(cellA, targetCell, nCellsSqrt);
            //fixme transform 7 in a parameter
            if (lmsDist > static_cast<long>(LIMIT_DIST) && !fpgaIsInvalidCell(lB, cB, nCellsSqrt) && snapId < 80 *
                totalSnapshots /
                100) {
                limitStrategyTrigger = true;
            }
#endif

            unicTry++;

            const bool IsBIoNode = g.nSuccV[b] == 0 || g.nPredV[b] == 0;

            if (IsBIoNode) {
                ioTries++;
            } else {
                clbTries++;
            }

            targetCell = -1;
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
            }
#ifdef SCAN_STRATEGY
            //fixme Transform 9 in a parameter!!!
            else if (snapId >= 80 * totalSnapshots / 100 && !scanned) {
                scanned = true;
                clbTries += static_cast<long>(nNodes - g.innerNodes.size());
                unicTry += static_cast<long>(nNodes - g.innerNodes.size());
                //scanning all cells and inserting the empty ones in the corresponding quadrant
                for (long l = 1; l < nCellsSqrt - 1; l++) {
                    for (long c = 1; c < nCellsSqrt - 1; c++) {
                        const long cell = l * nCellsSqrt + c;
                        if (c2n[cell].empty()) {
                            const long quadrant = getQuadrant(l, c, nCells, nCellsSqrt);
                            scannedCells[quadrant].push_back(cell);
                        }
                    }
                }
                continue;
            } else if (scanned) {
                if (!scannedCells[quadCounter].empty()) {
                    targetCell = scannedCells[quadCounter].back();
                    lB = targetCell / nCellsSqrt;
                    cB = targetCell % nCellsSqrt;
                    scannedCells[quadCounter].pop_back();
                } else {
                    quadCounter++;
                    if (quadCounter == 16) {
                        quadCounter = 0;
                    }
                    continue;
                }
            }
#endif

            else {
#ifdef LIMIT_STRATEGY
                long maxValue;
                pair<long, int> maxQuadrant;
                bool runYoto = true;

                if (limitStrategyTrigger && !abortLimitStrategy) {
                    const long quadrantA = getQuadrant(lA, cA, nCellsSqrt, nCellsSqrt);
                    std::vector<pair<long, int> > adjacentQuadrants = getAdjacentQuadrants(quadrantA);
                    maxQuadrant = adjacentQuadrants.front();
                    maxValue = limitQuadrants[maxQuadrant.first];
                    for (int i = 1; i < adjacentQuadrants.size(); i++) {
                        const pair<long, int> quadrantTmp = adjacentQuadrants[i];
                        const long valueTmp = limitQuadrants[quadrantTmp.first];
                        if (valueTmp > maxValue) {
                            maxValue = valueTmp;
                            maxQuadrant = quadrantTmp;
                        }
                    }
                    if (maxValue > 0) {
                        while (true) {
                            switch (maxQuadrant.second) {
                                case 0: // top
                                    lB = lA - limitAcc;
                                    cB = cA;
                                    break;
                                case 1: // bottom
                                    lB = lA + limitAcc;
                                    cB = cA;
                                    break;
                                case 2: // left
                                    lB = lA;
                                    cB = cA - limitAcc;
                                    break;
                                case 3: // right
                                    lB = lA;
                                    cB = cA + limitAcc;
                                    break;
                            }
                            isTargetCellIO = fpgaIsIOCell(lB, cB, nCellsSqrt);
                            const bool isInvalidCell = fpgaIsInvalidCell(lB, cB, nCellsSqrt);
                            // Check if the target cell is nor allowed, go to next
                            if (isInvalidCell || isTargetCellIO) {
                                abortLimitStrategy = true;
                                break;
                            }
                            targetCell = lB * nCellsSqrt + cB;
                            if (c2n[targetCell].empty()) {
                                runYoto = false;
                                break;
                            }
                            limitAcc++;
                            unicTry++;
                            clbTries++;
                        }
                    }
                }
                if (runYoto) {
#endif
                //Basic Spiral strategy
                lB = lA + ij[0];
                cB = cA + ij[1];

                isTargetCellIO = fpgaIsIOCell(lB, cB, nCellsSqrt);
                //prevents put a non IO node in an IO cell
                if (isTargetCellIO)
                    continue;

                //find the idx for the target cell
                targetCell = lB * nCellsSqrt + cB;
#ifdef LIMIT_STRATEGY
                }
#endif
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
#ifdef USE_CACHE
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
                distSlackCost += getManhattanDist(cellA, targetCell, nCellsSqrt) - 1 - g.slack[b];
#ifdef LIMIT_STRATEGY
                if (!isTargetCellIO) {
                    const long quadrant = getQuadrant(lB, cB, nCellsSqrt, nCellsSqrt);
                    limitQuadrants[quadrant]--;
                }
#endif
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
#ifdef PRINT_DOT
                fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
#ifdef PRINT_IMG
                //if (limitStrategyTrigger && !abortLimitStrategy)
                //writeMap(c2n, {n2c[a].first, n2c[b].first}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");
                if (snapTaken)
                    writeMap(c2n, {n2c[a].first, n2c[b].first}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");
                //fixme transform 9 in a parameter
                /*if (snapId >= 90 * 50 / 100)
                    writeMap(c2n, {n2c[a].first, n2c[b].first}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");*/
#endif
                long _max = 2 * dist * (dist + 1);
                if ((unicTry > _max) && !IsBIoNode
#ifdef SCAN_STRATEGY
                    && !scanned
#endif
#ifdef LIMIT_STRATEGY
                    && !limitStrategyTrigger
#endif
                ) {
                    cout << "Error while placing: dist=" << dist << " max tries:" << _max;
                    cout << ". Total tries" << unicTry;
                    //exit(1);
                }
#ifdef DEBUG
                placed = true;
#endif
                break;
            }
        }
#ifdef DEBUG
        if (!placed) {
            writeMap(c2n, {n2c[a].first, -1}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");
            int asfd = 1;
        }
#endif
    }

#ifdef MAKE_METRICS
    if (snapId < totalSnapshots)
        histogramFull.push_back(histogram);
#endif

#ifdef PRINT_DOT
    fpgaSavePlacedDot(n2c, c2n, g.gEdges, nCellsSqrt, "/home/jeronimo/placed.dot");
#endif
#ifdef PRINT_IMG
    writeMap(c2n, {-1, -1}, nCellsSqrt, "/home/jeronimo/tmp/placed.jpg");
#endif

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    auto _time = duration.count();


#ifdef FPGA_TOTAL_COST
    const long tc = fpgaCalcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
#elifdef FPGA_LONG_PATH_COST
    //fixme
    const long tc = 0; //fpgaCalcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
#elifdef FPGA_DISTANCE_SLACK_COST
    const long tc = distSlackCost;
#else
    const long tc = 0;
#endif

    const long nIOs = static_cast<long>(g.outputNodes.size() + g.inputNodes.size());
    const long tries = (clbTries + ioTries);

#ifdef CACHE
    const long cachePenalties = CACHE_W_PARAMETER * CACHE_W_COST * cacheMisses;
    constexpr const long cacheWParameter = CACHE_W_PARAMETER;
    constexpr const long cacheWCost = CACHE_W_CACHE_W_COST;
    const long triesP = tries + cachePenalties;
    const long triesPerNode = triesP / nNodes;
#else
    constexpr long cachePenalties = 0;
    constexpr const long cacheWParameter = 0;
    constexpr const long cacheWCost = 0;
    constexpr long triesP = 0;
    const long triesPerNode = tries / nNodes;
#endif

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
        cacheWParameter,
        cacheWCost,
        cachePenalties,
        clbTries,
        ioTries,
        tries,
        triesP,
        triesPerNode,
        swaps,
        alg_type,
        costStrategyName,
        tc,
        c2n,
        n2c,
        histogramFull,
        heatEnd,
        heatBegin,
        originDestin
    );
    return report;
}
