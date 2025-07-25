#include <../../include/fpga/parametersFpga.h>
#include <common/cache.h>
#include <qca/qcaYott.h>


QcaReportData qcaYott(QCAGraph &g) {
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;
    const vector<pair<long, long> > ed = g.gEdges;

    vector<long> c2n(nCells, -1);
    vector<long> n2c(nNodes, -1);

    vector<long> cells(nCells);
#ifdef USE
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);
#elifdef WAVE2D
    vector<long> outCells = g.getInterleavedOutputCellsQCA();
#endif

#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif

    long tries = 0;
    long swaps = 0;


    string alg_type;

    vector<pair<long, long> > convergence;
    vector<tuple<long, long, string> > ed_zz;
    const vector ed_tmp = g.getEdgesZigzag(convergence, &ed_zz);

    unordered_map<string, vector<pair<long, long> > > annotations = g.qcaGetGraphAnnotations(ed_tmp, convergence);

    alg_type = "ZIG_ZAG";


    auto start = chrono::high_resolution_clock::now();

    for (const auto &[a,b,dir]: ed_zz) {
        //Verify if A is placed
        //if it is not placed, then place in a random unused cell.
        //the variable lastCellIdxUsed is for optimize future looks

        if (n2c[a] == -1) {
            bool found = false;
            while (!found) {
                long cell = cells.back();
                cells.pop_back();
                if (c2n[cell] == -1) {
                    c2n[cell] = a;
                    n2c[a] = cell;
                    found = true;
                }
            }
        }

#ifdef PRINT
        //qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif

        //Now, if B is placed, go to next edge
        if (n2c[b] != -1)
            continue;


        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const long xA = getX(n2c[a], nCellsSqrt);
        const long yA = getY(n2c[a], nCellsSqrt);

        vector<pair<long, long> > distCells;

        if (dir == "IN")
            distCells = qcaGetInputDirections(xA, yA);
        else
            distCells = qcaGetOutputDirections(xA, yA);

        randomVector(distCells);

        long betterCell = -1;
        long betterCellDist = nCells;

        bool placed = false;

        //Then I will look for a cell next to A's cell
        for (const auto &[fst,snd]: distCells) {
            ++tries;
            const long yB = yA + snd;
            const long xB = xA + fst;

            //find the idx for the target cell
            const long targetCell = yB * nCellsSqrt + xB;

            // Check if the target cell is not allowed, go to next
            if (qcaIsInvalidCell(xB, yB, nCellsSqrt))
                continue;

            //begin of yott verifications.
            // if yott verifications does not match until targetCellDistance < 4, then
            //the better found cell will be used and if it is not set, yoto will be executed
            //the code needs to be improved, but ir works

            //beginning with an annotation if it exists
            const string key = to_string(a) + " " + to_string(b);
            const vector<pair<long, long> > annotation = annotations[key];

            //if we have an annotation
            if (!annotation.empty()) {
                if (c2n[targetCell] != -1)
                    continue;

                long modDist = 0;
                bool found = true;
                //find the distance of the target cell to the annotated cell and compare if they are equal
                for (auto &[fst, snd]: annotation) {
                    const long annCell = n2c[fst];
                    const long annDist = snd + 1;
                    const long tAnnDist = getManhattanDist(targetCell, annCell, nCellsSqrt);

                    modDist += abs(annDist - tAnnDist);
                }
                if (modDist < betterCellDist && c2n[targetCell] == -1) {
                    betterCellDist = modDist;
                    betterCell = targetCell;
                }
                continue;
            }
            //if edge has no annotations
            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                placed = true;
                break;
            }
        }
        if (betterCell > -1) {
            c2n[betterCell] = b;
            n2c[b] = betterCell;
            ++swaps;
        } else if (!placed)
            break; //not placed
    }
#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif

    const auto end = chrono::high_resolution_clock::now();
    const chrono::duration<double, milli> duration = end - start;
    double _time = duration.count();

    //if this placement valid?
    long wrongEdges;
    bool success = g.verifyPlacement(n2c, ed, &wrongEdges);
    bool nodesPlaced = allPlaced(n2c);
    AreaMetrics saMetrics = computeOccupiedAreaMetrics(nCellsSqrt, c2n);


    auto report = QcaReportData(
        success,
        nodesPlaced,
        static_cast<float>(_time),
        g.dotName,
        g.dotPath,
        "SA",
        nCellsSqrt,
        static_cast<long>(g.dummyMap.size()),
        static_cast<long>(g.nNodes - g.dummyMap.size()),
        tries,
        swaps,
        wrongEdges,
        saMetrics.totalCells,
        saMetrics.utilization,
        g.extraLayers,
        g.extraLayersLevels,
        c2n,
        n2c,
        ed
    );
    return report;
}
