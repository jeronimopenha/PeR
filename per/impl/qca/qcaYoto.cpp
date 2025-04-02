#include <qca/qcaYoto.h>
#include <common/parameters.h>
#include <vector>


QcaReportData qcaYoto(QCAGraph& g)
{
    const int nCells = g.nCells;
    int nCellsSqrt = g.nCellsSqrt;
    const int nNodes = g.nNodes;

    vector<int> c2n(nCells, -1);
    vector<int> n2c(nNodes, -1);

    vector<int> cells(nCells);
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);

#ifdef DEBUG
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, nCellsSqrt);
#endif

    int tries = 0;
    int swaps = 0;

    string alg_type;
    vector<pair<int, int>> ed;

#ifdef QCA_YOTO_DF
    ed = g.getEdgesDepthFirst();
    alg_type = "DEPTH_FIRST";
#endif

    int lastCellIdxUsed = 0;

    //time counting
    const auto start = chrono::high_resolution_clock::now();

    for (auto [a,b] : ed)
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

#ifdef DEBUG
        qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, nCellsSqrt);
#endif

        //Now, if B is placed, go to next edge
        if (n2c[b] != -1)
        {
            continue;
        }

        // Now I will try to find an adjacent cell from A to place B

        // Find the idx of A's cell
        const int xA = getX(n2c[a], nCellsSqrt);
        const int yA = getY(n2c[a], nCellsSqrt);

        vector<pair<int, int>> distCells = qcaGetOutputDirections(xA, yA);
        bool placed = false;
        //Then I will look for a cell next to A's cell
        for (const auto& dist : distCells)
        {
            ++tries;
            const int xB = xA + dist.first;
            const int yB = yA + dist.second;

            //find the idx for the target cell
            int targetCell = yB * nCellsSqrt + xB;

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
#ifdef DEBUG
                qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, nCellsSqrt);
#endif
                break;
            }
        }
        if (!placed)
            break;
    }

    /*const auto end = chrono::high_resolution_clock::now();
    const chrono::duration<double, milli> duration = end - start;
    double _time = duration.count();

    int tc = 0;
    // commented to take the cost of the longest path
#ifdef FPGA_TOTAL_COST
    tc = calcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
#elifdef FPGA_LP_COST
    tc = calcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
#endif

    ReportData report = ReportData(
        _time,
        g.dotName,
        g.dotPath,
        "yoto",
        cacheMisses,
        tries,
        swaps,
        alg_type,
        tc,
        c2n,
        n2c
    );
    return report;
    */
}
