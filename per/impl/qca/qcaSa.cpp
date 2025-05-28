#include <common/parameters.h>
#include <qca/qcaSa.h>
#include <unordered_set>

QcaReportData qcaSa(QCAGraph &g) {
    const string alg_type = "SA";
    long tries = 0;
    long swaps = 0;

    const long nCells = g.nCells;
    const long nCellsSqrt = g.nCellsSqrt;
    const long nNodes = g.nNodes;
    const vector<pair<long, long> > ed = g.gEdges;

    vector<vector<long> > c2n(nCells, vector<long>(2, -1));
    vector<vector<long> > n2c(nNodes, vector<long>(2, -1));

    vector<long> cells(nCells);
    iota(cells.begin(), cells.end(), 0);
    randomVector(cells);


    //place the nodes to their initial positions
    long idx = 0;
    for (long node = 0; node < nNodes; node++)
        if (c2n[cells[idx]][0] == -1)
        {
            c2n[cells[idx]][0] = node;
            n2c[node][0] = cells[idx];
            n2c[node][1] = 0L;
            idx++;
        }


#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif
/*
    //begin of SA algorithm
    constexpr float t_min = 0.001f;
    float t = 100;
    bool success = false;

    auto start = chrono::high_resolution_clock::now();

    while (t >= t_min)
    {
        for (long cellA = 0; cellA < nCells; cellA++)
        {
            for (long cellB = 0; cellB < nCells; cellB++)
            {
                tries++;

                if (cellA == cellB)
                    continue;

                const long a = c2n[cellA];
                const long b = c2n[cellB];

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

                if ((costAfter < costBefore) || (rnd <= value))
                {
                    if (a != -1)
                    {
                        n2c[a] = cellB;
                    }
                    if (b != -1)
                    {
                        n2c[b] = cellA;
                    }
                    c2n[cellA] = b;
                    c2n[cellB] = a;
                    swaps++;
                    success = g.verifyPlacement(n2c, ed);

#ifdef PRINT
                    //qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif
                }
                if (success)
                    break;
            }
            if (success)
                break;
#ifdef PRINT
            if (t <= 1)
            {
                //qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
            }
#endif
            t *= 0.999;
        }
        if (success)
            break;
    }
#ifdef PRINT
    qcaExportUSEToDot("/home/jeronimo/use.dot", n2c, ed, nCellsSqrt);
#endif
    const auto end = chrono::high_resolution_clock::now();
    const chrono::duration<double, milli> duration = end - start;
    double _time = duration.count();

    //if this placement valid?
    long wrongEdges;
    success = g.verifyPlacement(n2c, ed, &wrongEdges);
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

    return report;*/
}


QcaCost computeQcaCostForNode(
    const long node,
    const long nodeCell,
    const long otherNode,
    const long otherCell,
    const vector<long> &n2c,
    const QCAGraph &g
) {
    QcaCost cost;

    if (node == -1) return cost;

    const long nCellsSqrt = g.nCellsSqrt;
    const long x = getX(nodeCell, nCellsSqrt);
    const long y = getY(nodeCell, nCellsSqrt);

    const auto inDirs = qcaGetInputDirections(x, y);
    const auto outDirs = qcaGetOutputDirections(x, y);

    unordered_set<long> expectedInputs, expectedOutputs;
    for (auto [dx, dy]: inDirs) {
        const long cx = x + dx;
        const long cy = y + dy;
        if (!qcaIsInvalidCell(cx, cy, nCellsSqrt))
            expectedInputs.insert(getCellIndex(cx, cy, nCellsSqrt));
    }
    for (auto [dx, dy]: outDirs) {
        const long cx = x + dx;
        const long cy = y + dy;
        if (!qcaIsInvalidCell(cx, cy, nCellsSqrt))
            expectedOutputs.insert(getCellIndex(cx, cy, nCellsSqrt));
    }

    // Check inputs
    for (const long pred: g.inNeighbors[node]) {
        const long cell = (pred == otherNode) ? otherCell : n2c[pred];
        if (cell == -1)
            continue;

        if (expectedInputs.count(cell)) {
            const long dx = abs(getX(cell, nCellsSqrt) - x);
            const long dy = abs(getY(cell, nCellsSqrt) - y);
            const long dist = dx + dy;
            if (dist == 1)
                cost.bonus += 1.0f;
            else
                cost.distancePenalty += static_cast<float>(min(dist - 1, 3l));
        } else
            cost.directionPenalty += 3.0f;
    }

    // Check outputs
    for (const long succ: g.outNeighbors[node]) {
        const long cell = (succ == otherNode) ? otherCell : n2c[succ];
        if (cell == -1)
            continue;

        if (expectedOutputs.count(cell)) {
            const long dx = abs(getX(cell, nCellsSqrt) - x);
            const long dy = abs(getY(cell, nCellsSqrt) - y);
            const long dist = dx + dy;
            if (dist == 1)
                cost.bonus += 1.0f;
            else
                cost.distancePenalty += static_cast<float>(min(dist - 1, 3L));
        } else
            cost.directionPenalty += 3.0f;
    }

    // Penalty for direct inversion
    if (otherNode != -1)
        if ((g.successors[node][otherNode] || g.successors[otherNode][node]) && node != otherNode)
            cost.inversionPenalty += 2.0f; // adjustable

    return cost;
}

// Function to calculate total cost of exchange
tuple<float, float> qcaGetSwapCostImproved(
    const long cellA, const long cellB,
    const long nodeA, const long nodeB,
    const vector<long> &n2c,
    const QCAGraph &g
) {
    if (nodeA == -1) return {0.0f, 0.0f};

    const auto costABefore = computeQcaCostForNode(nodeA, cellA, nodeB, cellB, n2c, g);
    const auto costBBefore = computeQcaCostForNode(nodeB, cellB, nodeA, cellA, n2c, g);
    const auto costAAfter = computeQcaCostForNode(nodeA, cellB, nodeB, cellA, n2c, g);
    const auto costBAfter = computeQcaCostForNode(nodeB, cellA, nodeA, cellB, n2c, g);

    float before = costABefore.total() + costBBefore.total();
    float after = costAAfter.total() + costBAfter.total();

    // Normalize between 0 and 1 assuming realistic maximum cost per node
    const size_t totalNeigh = g.inNeighbors[nodeA].size() + g.outNeighbors[nodeA].size();
    const float normFactor = static_cast<float>(std::max<size_t>(totalNeigh, 1)) * 3.0f;
    //const float normFactor = static_cast<float>(std::max<size_t>(totalNeigh, 1)) * 4.0f;
    //const float normFactor = static_cast<float>(std::max<size_t>(totalNeigh, 1)) * 6.0f;
    before = before / normFactor;
    after = after / normFactor;

    return {before * EXPLORE_FACTOR, after * EXPLORE_FACTOR};
}
