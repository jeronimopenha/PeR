#include <qca/qcaYoto.h>
#include <common/parameters.h>
#include <vector>


QcaReportData qcaYoto(QCAGraph& g)
{
    const int nCells = g.nCells;
    const int nCellsSqrt = g.nCellsSqrt;
    const int nNodes = g.nNodes;
    const vector<pair<int, int>> ed = g.gEdges;

    vector c2n(nCells, -1);
    vector n2c(nNodes, -1);

    vector<int> cells(nCells);
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);

#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif

    int tries = 0;
    int swaps = 0;

    string alg_type;

    vector<pair<int, int>> convergence;
    vector<tuple<int, int, string>> ed_zz;
    g.getEdgesZigzag(convergence, &ed_zz);
    alg_type = "ZIGZAG";

    int lastCellIdxUsed = 0;

    //time counting
    const auto start = chrono::high_resolution_clock::now();

    for (const auto& [a,b,dir] : ed_zz)

    {
        //Verify if A is placed
        //if it is not placed, then place in a random unused cell.
        //the variable lastCellIdxUsed is for optimize future looks
        if (n2c[a] == -1)
        {
            bool found = false;
            while (!found)
            {
                int cell = cells[lastCellIdxUsed];
                while (cell == -1)
                {
                    lastCellIdxUsed++;
                    cell = cells[lastCellIdxUsed];
                }
                if (c2n[cell] == -1)
                {
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
        const int xA = getX(n2c[a], nCellsSqrt);
        const int yA = getY(n2c[a], nCellsSqrt);

        vector<pair<int, int>> distCells;

        if (dir == "IN")
            distCells = qcaGetInputDirections(xA, yA);
        else
            distCells = qcaGetOutputDirections(xA, yA);

        randomVector(distCells);

        bool placed = false;
        //Then I will look for a cell next to A's cell
        for (const auto& [fst, snd] : distCells)
        {
            ++tries;
            const int xB = xA + fst;
            const int yB = yA + snd;

            //find the idx for the target cell
            const int targetCell = yB * nCellsSqrt + xB;

            // Check if the target cell is nor allowed, go to next
            if (qcaIsInvalidCell(xB, yB, nCellsSqrt))
                continue;

            // Place the node if `placement[targetCell]` is unoccupied
            if (c2n[targetCell] == -1)
            {
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
    int wrongEdges;
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
        static_cast<int>(g.dummyMap.size()),
        static_cast<int>(g.nNodes - g.dummyMap.size()),
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
