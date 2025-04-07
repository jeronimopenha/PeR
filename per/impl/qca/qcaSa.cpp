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
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif

    //begin of SA algorithm
    const float t_min = 0.001;
    float t = 100;
    bool success = false;

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

                //auto [costABefore, costAAfter] = qcaGetSwapCost(cellA, cellB, a, b, n2c, g);
                //auto [costBBefore, costBAfter] = qcaGetSwapCost(cellB, cellA, b, a, n2c, g);

                auto [costABefore, costAAfter] = qcaGetSwapCostImproved(cellA, cellB, a, b, n2c, g);
                auto [costBBefore, costBAfter] = qcaGetSwapCostImproved(cellB, cellA, b, a, n2c, g);

                const float costBefore = (costABefore + costBBefore);
                const float costAfter = (costAAfter + costBAfter);

                const auto delta = costAfter - costBefore;
                const double value = exp(-1 * delta / t);

                const double rnd = randomFloat(0.0f, 1.0f);

                if ((costAfter < costBefore) || (rnd <= value)) {
                    if (a != -1) {
                        n2c[a] = cellB;
                    }
                    if (b != -1) {
                        n2c[b] = cellA;
                    }
                    c2n[cellA] = b;
                    c2n[cellB] = a;
                    swaps++;
                    success = g.verifyPlacement(n2c,ed);
                    if (success) {
                        int asdf = 1;
                    }
#ifdef PRINT
                    //qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif
                }
                if (success) break;
            }
            if (success) break;
#ifdef PRINT
            if (t <= 1) {
                //qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
            }
#endif
            t *= 0.999;
        }
        if (success) break;
    }
#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif
    const auto end = chrono::high_resolution_clock::now();
    const chrono::duration<double, milli> duration = end - start;
    double _time = duration.count();

    //if this placement valid?
    int wrongEdges;
    success = g.verifyPlacement(n2c,ed,&wrongEdges);
    AreaMetrics saMetrics = computeOccupiedAreaMetrics(nCellsSqrt,c2n);


    auto report = QcaReportData(
        success,
        _time,
        g.dotName,
        g.dotPath,
        "SA",
        nCellsSqrt,
        g.dummyMap.size(),
        g.nNodes - g.dummyMap.size(),
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
            //costB += dist; // ou: costB += dist;
            //costB += 1; // ou: costB += dist;
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
            //costB += dist; // ou: costB += dist;
            //costB += 1; // ou: costB += dist;
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
            //costA += dist; // ou: costB += dist;
            //costA += 1; // ou: costB += dist;
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
            //costA += dist; // ou: costB += dist;
            //costA += 1; // ou: costB += dist;
        }
    }
    //float totalCostB = static_cast<float>(costB) / static_cast<float>(totalNeigh * (2 * nCellsSqrt));
    //float totalCostA = static_cast<float>(costA) / static_cast<float>(totalNeigh * (2 * nCellsSqrt));

    float totalCostB = static_cast<float>(costB) / static_cast<float>(totalNeigh * 3);
    float totalCostA = static_cast<float>(costA) / static_cast<float>(totalNeigh * 3);

    return {totalCostB, totalCostA};
    //return {costB, costA};
}

QcaCost computeQcaCostForNode(
    const int node,
    const int nodeCell,
    const int otherNode,
    const int otherCell,
    const vector<int> &n2c,
    const QCAGraph &g
) {
    QcaCost cost;

    if (node == -1) return cost;

    const int nCellsSqrt = g.nCellsSqrt;
    const int x = getX(nodeCell, nCellsSqrt);
    const int y = getY(nodeCell, nCellsSqrt);

    const auto inDirs = qcaGetInputDirections(x, y);
    const auto outDirs = qcaGetOutputDirections(x, y);

    unordered_set<int> expectedInputs, expectedOutputs;
    for (auto [dx, dy]: inDirs) {
        const int cx = x + dx;
        const int cy = y + dy;
        if (!qcaIsInvalidCell(cx, cy, nCellsSqrt)) {
            expectedInputs.insert(getCellIndex(cx, cy, nCellsSqrt));
        }
    }
    for (auto [dx, dy]: outDirs) {
        int cx = x + dx, cy = y + dy;
        if (!qcaIsInvalidCell(cx, cy, nCellsSqrt)) {
            expectedOutputs.insert(getCellIndex(cx, cy, nCellsSqrt));
        }
    }

    // Verifica entradas
    for (int pred: g.inNeighbors[node]) {
        int cell = (pred == otherNode) ? otherCell : n2c[pred];
        if (cell == -1) continue;

        if (expectedInputs.count(cell)) {
            int dx = abs(getX(cell, nCellsSqrt) - x);
            int dy = abs(getY(cell, nCellsSqrt) - y);
            int dist = dx + dy;
            if (dist == 1)
                cost.bonus += 1.0f;
            else
                cost.distancePenalty += min(dist - 1, 3);
        } else {
            cost.directionPenalty += 3.0f;
        }
    }

    // Verifica saídas
    for (int succ: g.outNeighbors[node]) {
        int cell = (succ == otherNode) ? otherCell : n2c[succ];
        if (cell == -1) continue;

        if (expectedOutputs.count(cell)) {
            int dx = abs(getX(cell, nCellsSqrt) - x);
            int dy = abs(getY(cell, nCellsSqrt) - y);
            int dist = dx + dy;
            if (dist == 1)
                cost.bonus += 1.0f;
            else
                cost.distancePenalty += min(dist - 1, 3);
        } else {
            cost.directionPenalty += 3.0f;
        }
    }

    // Penalidade por inversão direta
    if (otherNode != -1)
        if ((g.successors[node][otherNode] || g.successors[otherNode][node]) && node != otherNode) {
            cost.inversionPenalty += 2.0f; // ajustável
        }

    return cost;
}

// Função para calcular custo total da troca
tuple<float, float> qcaGetSwapCostImproved(
    int cellA, int cellB,
    int nodeA, int nodeB,
    const vector<int> &n2c,
    const QCAGraph &g
) {
    if (nodeA == -1) return {0.0f, 0.0f};

    auto costABefore = computeQcaCostForNode(nodeA, cellA, nodeB, cellB, n2c, g);
    auto costBBefore = computeQcaCostForNode(nodeB, cellB, nodeA, cellA, n2c, g);
    auto costAAfter = computeQcaCostForNode(nodeA, cellB, nodeB, cellA, n2c, g);
    auto costBAfter = computeQcaCostForNode(nodeB, cellA, nodeA, cellB, n2c, g);

    float before = costABefore.total() + costBBefore.total();
    float after = costAAfter.total() + costBAfter.total();

    // Normaliza entre 0 e 1 assumindo custo máximo realista por nó
    size_t totalNeigh = g.inNeighbors[nodeA].size() + g.outNeighbors[nodeA].size();
    float normFactor = static_cast<float>(std::max<size_t>(totalNeigh, 1)) * 4.0f;
    before = before / normFactor;
    after = after / normFactor;

    return {before, after};
}
