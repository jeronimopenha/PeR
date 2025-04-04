#include <qca/qcaSa.h>

QcaReportData qcaSa(QCAGraph &g) {
    const string alg_type = "SA";
    int tries = 0;
    int swaps = 0;

    const int nCells = g.nCells;
    const int nCellsSqrt = g.nCellsSqrt;
    const int nNodes = g.nNodes;
    const vector<pair<int, int> > ed = g.gEdges;

    vector<int> c2n(nCells, -1);
    vector<int> n2c(nNodes, -1);

    vector<int> cells(nCells);
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);

    //place the nodes to their initial positions
    int idx = 0;
    for (int node = 0; node < nNodes; node++) {
        if (c2n[cells[idx]] == -1) {
            c2n[cells[idx]] = node;
            n2c[node] = cells[idx];
            idx++;
        }
    }


#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, nCellsSqrt);
#endif

    //begin of SA algorithm
    const float t_min = 0.001;
    float t = 100;

    auto start = chrono::high_resolution_clock::now();

    while (t >= t_min) {
        for (int cellA = 0; cellA < nCells; cellA++) {
            for (int cellB = 0; cellB < nCells; cellB++) {
                tries++;

                if (cellA == cellB)
                    continue;

                const int a = c2n[cellA];
                const int b = c2n[cellB];

                if (a == -1 && b == -1)
                    continue;

                auto [costABefore, costAAfter] = qcaGetSwapCost(cellA, cellB, a, b, n2c, g);
                auto [costBBefore, costBAfter] = qcaGetSwapCost(cellB, cellA, b, a, n2c, g);

                const float costBefore = costABefore + costBBefore;
                const float costAfter = costAAfter + costBAfter;

                const auto delta = costAfter - costBefore;
                const double value = exp((-delta / t));

                const double rnd = randomFloat(0.0f, 1.0f);

                if ((costAfter < costBefore)){// || (rnd <= value)) {
                    if (a != -1) {
                        n2c[a] = cellB;
                    }
                    if (b != -1) {
                        n2c[b] = cellA;
                    }
                    c2n[cellA] = b;
                    c2n[cellB] = a;
                    swaps++;
                }
            }

            t *= 0.999;
        }
    }
#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, nCellsSqrt);
#endif
    const auto end = chrono::high_resolution_clock::now();
    const chrono::duration<double, milli> duration = end - start;
    double _time = duration.count();

    //if this placement valid?

    bool success = g.verifyPlacement(n2c);

    auto report = QcaReportData(
        success,
        _time,
        g.dotName,
        g.dotPath,
        "YOTO",
        g.dummyMap.size(),
        g.nNodes - g.dummyMap.size(),
        tries,
        swaps,
        g.extraLayers,
        g.extraLayersLevels,
        c2n,
        n2c
    );
    return report;
}


pair<float, float> qcaGetSwapCost(
    const int cellNode1,
    const int cellNode2,
    const int node1,
    const int node2,
    const vector<int> &n2c,
    const QCAGraph &g
) {
    if (node1 == -1) {
        return {0.0, 0.0};
    }
    const int nCellsSqrt = g.nCellsSqrt;
    const int totalNeigh = static_cast<int>(g.inNeighbors[node1].size() + g.outNeighbors[node1].size());

    int costB = 0;
    int costA = 0;

    //todo add a minor cost penalty if neigh is near node1 but in a wrong cell
    vector<int> targetNodes;

    //cost before
    int cellLookX = getX(cellNode1, nCellsSqrt);
    int cellLookY = getY(cellNode1, nCellsSqrt);;

    vector<int> inCellsNode1;
    vector<int> outCellsNode1;

    for (auto [x,y]: qcaGetInputDirections(cellLookX, cellLookY)) {
        const int cellX = cellLookX + x;
        const int cellY = cellLookY + y;
        if (qcaIsInvalidCell(cellX, cellY, nCellsSqrt)) continue;
        const int cell = getCellIndex(cellX, cellY, nCellsSqrt);
        inCellsNode1.push_back(cell);
    }

    for (auto [x,y]: qcaGetOutputDirections(cellLookX, cellLookY)) {
        const int cellX = cellLookX + x;
        const int cellY = cellLookY + y;
        if (qcaIsInvalidCell(cellX, cellY, nCellsSqrt)) continue;
        const int cell = getCellIndex(cellX, cellY, nCellsSqrt);
        outCellsNode1.push_back(cell);
    }

    for (const auto neigh: g.inNeighbors[node1]) {
        //look if neigh is in a valid cell with A
        const int nCell = n2c[neigh];
        bool found = false;
        for (const int cell: inCellsNode1)
            if (nCell == cell) {
                found = true;
                break;
            }
        if (!found) {
            //costB++;
            int nx = getX(nCell, nCellsSqrt);
            int ny = getY(nCell, nCellsSqrt);
            int dx = abs(nx - cellLookX);
            int dy = abs(ny - cellLookY);
            int dist = dx + dy;

            // Penaliza 1.0 se estiver fora da direção esperada
            // Penaliza parcialmente se estiver na direção mas distante
            costB += min(dist, 3); // ou: costB += dist;
        }
    }

    for (const auto neigh: g.outNeighbors[node1]) {
        //look if neigh is in a valid cell with A
        const int nCell = n2c[neigh];
        bool found = false;
        for (const int cell: outCellsNode1)
            if (nCell == cell) {
                found = true;
                break;
            }
        if (!found) {
            //costB++;
            int nx = getX(nCell, nCellsSqrt);
            int ny = getY(nCell, nCellsSqrt);
            int dx = abs(nx - cellLookX);
            int dy = abs(ny - cellLookY);
            int dist = dx + dy;

            // Penaliza 1.0 se estiver fora da direção esperada
            // Penaliza parcialmente se estiver na direção mas distante
            costB += min(dist, 3); // ou: costB += dist;
        }
    }

    //cost after
    cellLookX = getX(cellNode2, nCellsSqrt);
    cellLookY = getY(cellNode2, nCellsSqrt);;

    inCellsNode1.clear();
    outCellsNode1.clear();

    for (auto [x,y]: qcaGetInputDirections(cellLookX, cellLookY)) {
        const int cellX = cellLookX + x;
        const int cellY = cellLookY + y;
        if (qcaIsInvalidCell(cellX, cellY, nCellsSqrt)) continue;
        const int cell = getCellIndex(cellX, cellY, nCellsSqrt);
        inCellsNode1.push_back(cell);
    }

    for (auto [x,y]: qcaGetOutputDirections(cellLookX, cellLookY)) {
        const int cellX = cellLookX + x;
        const int cellY = cellLookY + y;
        if (qcaIsInvalidCell(cellX, cellY, nCellsSqrt)) continue;
        const int cell = getCellIndex(cellX, cellY, nCellsSqrt);
        outCellsNode1.push_back(cell);
    }

    for (const auto neigh: g.inNeighbors[node1]) {
        //look if neigh is in a valid cell with A
        const int nCell = (neigh == node2) ? cellNode1 : n2c[neigh];

        bool found = false;
        for (const int cell: inCellsNode1)
            if (nCell == cell) {
                found = true;
                break;
            }
        if (!found) {
            //costA++;
            int nx = getX(nCell, nCellsSqrt);
            int ny = getY(nCell, nCellsSqrt);
            int dx = abs(nx - cellLookX);
            int dy = abs(ny - cellLookY);
            int dist = dx + dy;

            // Penaliza 1.0 se estiver fora da direção esperada
            // Penaliza parcialmente se estiver na direção mas distante
            costA += min(dist, 3); // ou: costB += dist;
        }
    }

    for (const auto neigh: g.outNeighbors[node1]) {
        //look if neigh is in a valid cell with A
        const int nCell = (neigh == node2) ? cellNode1 : n2c[neigh];
        bool found = false;
        for (const int cell: outCellsNode1)
            if (nCell == cell) {
                found = true;
                break;
            }
        if (!found) {
            //costA++;
            int nx = getX(nCell, nCellsSqrt);
            int ny = getY(nCell, nCellsSqrt);
            int dx = abs(nx - cellLookX);
            int dy = abs(ny - cellLookY);
            int dist = dx + dy;

            // Penaliza 1.0 se estiver fora da direção esperada
            // Penaliza parcialmente se estiver na direção mas distante
            costA += min(dist, 3); // ou: costB += dist;
        }
    }
    float totalCostB = static_cast<float>(costB) / static_cast<float>(totalNeigh * 3);
    float totalCostA = static_cast<float>(costA) / static_cast<float>(totalNeigh * 3);

    return {totalCostB, totalCostA};
}
