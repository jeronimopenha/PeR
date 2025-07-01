#include <qca/qcaYoto.h>
#include <common/parameters.h>
#include <vector>


QcaReportData qcaYoto(QCAGraph &g) {
    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;
    const vector<pair<long, long> > ed = g.gEdges;

    vector<long> c2n(nCells, -1);
    vector<long> n2c(nNodes, -1);

    vector<long> cells(nCells);
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);

#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif

    long tries = 0;
    long swaps = 0;

    string alg_type;

    vector<pair<long, long> > convergence;
    vector<tuple<long, long, string> > ed_zz;
    g.getEdgesZigzag(convergence, &ed_zz);
    alg_type = "ZIGZAG";

    long lastCellIdxUsed = 0;

    //time counting
    const auto start = chrono::high_resolution_clock::now();

    for (const auto &[a,b,dir]: ed_zz) {
        //Verify if A is placed
        //if it is not placed, then place in a random unused cell.
        //the variable lastCellIdxUsed is for optimize future looks
        if (n2c[a] == -1) {
            bool found = false;
            while (!found) {
                long cell = cells[lastCellIdxUsed];
                while (cell == -1) {
                    lastCellIdxUsed++;
                    cell = cells[lastCellIdxUsed];
                }
                if (c2n[cell] == -1) {
                    c2n[cell] = a;
                    n2c[a] = cell;
                    cells[lastCellIdxUsed] = -1;
                    lastCellIdxUsed++;
                    found = true;
                }
            }
        }

#ifdef PRINT
        qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
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

        bool placed = false;
        //Then I will look for a cell next to A's cell
        for (const auto &[fst, snd]: distCells) {
            ++tries;
            const long xB = xA + fst;
            const long yB = yA + snd;

            //find the idx for the target cell
            const long targetCell = yB * nCellsSqrt + xB;

            // Check if the target cell is nor allowed, go to next
            if (qcaIsInvalidCell(xB, yB, nCellsSqrt))
                continue;

            // Place the node if `placement[targetCell]` is unoccupied
            if (c2n[targetCell] == -1) {
                c2n[targetCell] = b;
                n2c[b] = targetCell;
                ++swaps;
                placed = true;
#ifdef PRINT
                qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif
                break;
            }
        }
        if (!placed)
            break;
    }


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
