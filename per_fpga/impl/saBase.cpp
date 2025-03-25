#include "saBase.h"


#include "yottBase.h"


ReportData saBase(Graph& g)
{
    const string alg_type = "SA";
    int cacheMisses = 0;
    int tries = 0;
    int swaps = 0;

    const int nCells = g.nCells;
    const int nCellsSqrt = g.nCellsSqrt;
    const int nNodes = g.nNodes;
    const vector<pair<int, int>> ed = g.gEdges;


    vector<int> c2n(nCells, -1);
    vector<int> n2c(nNodes, -1);

    vector<int> inOutCells = g.getInOutPos();
    randomVector(inOutCells);

    vector<int> clbCells = g.getClbPos();
    randomVector(clbCells);

    //place the io nodes to their initial positions
    vector<int> ioNodes;
    ioNodes.reserve(g.inputNodes.size() + g.outputNodes.size());
    ioNodes.insert(ioNodes.end(), g.inputNodes.begin(), g.inputNodes.end());
    ioNodes.insert(ioNodes.end(), g.outputNodes.begin(), g.outputNodes.end());


    int idx = 0;
    for (int node : ioNodes)
    {
        if (c2n[inOutCells[idx]] == -1)
        {
            c2n[inOutCells[idx]] = node;
            n2c[node] = inOutCells[idx];
            idx++;
        }
    }

    //place the clb nodes to their initial positions
    vector<int> clbNodes = g.clbNodes;

    idx = 0;
    for (int node : clbNodes)
    {
        if (c2n[clbCells[idx]] == -1)
        {
            c2n[clbCells[idx]] = node;
            n2c[node] = clbCells[idx];
            idx++;
        }
    }

    std::vector<std::vector<int>> neighbors = g.neighbors;

    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<> dis(0.0, 1.0);

    //savePlacedDot(n2c, ed, nCellsSqrt, "/home/jeronimo/placed.dot");

    //begin of SA algorithm
    const float t_min = 0.001;
    float t = 100;

    auto start = chrono::high_resolution_clock::now();

    while (t >= t_min)
    {
        for (int cellA = 1; cellA < nCells; cellA++)
        {
            for (int cellB = 1; cellB < nCells; cellB++)
            {
                tries++;

                if (cellA == cellB)
                    continue;
                // Check if cellA is nor allowed, go to next
                if (is_invalid_cell(cellA, nCellsSqrt))
                    continue;
                if (is_invalid_cell(cellB, nCellsSqrt))
                    continue;

                //prevents IO nodes to be not put in IO cells
                //and put a non IO node in an IO cell
                const bool isCellAIO = is_io_cell(cellA, nCellsSqrt);
                const bool isCellBIO = is_io_cell(cellB, nCellsSqrt);

                if ((isCellAIO && !isCellBIO) || (!isCellAIO && isCellBIO))
                    continue;

                int a = c2n[cellA];
                int b = c2n[cellB];

                if (a == -1 && b == -1)
                    continue;

                int costABefore, costAAfter;
                int costBBefore, costBAfter;

                getSwapCost(n2c, a, b, cellA, cellB, nCellsSqrt, neighbors, costABefore, costAAfter, costABefore,
                            costBAfter);

                int costAfter = costAAfter + costBAfter;
                int costBefore = costABefore + costBBefore;

                double value = exp((-1 * (costAfter - costBefore) / t));

                double rnd = dis(gen);

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
                    //savePlacedDot(n2c, ed, nCellsSqrt, "/home/jeronimo/placed.dot");
                }
            }
            t *= 0.999;
        }
    }

    //savePlacedDot(n2c, ed, nCellsSqrt, "/home/jeronimo/placed.dot");

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    float _time = duration.count();

    // commented to take the cost of the longest path
#ifdef TOTAL_COST
    int tc = calcGraphTotalDistance(n2c, g.gEdges, nCellsSqrt);
#elifdef LP_COST
    int tc = calcGraphLPDistance(g.longestPath, n2c, nCellsSqrt);
#endif

    ReportData report = ReportData(
        _time,
        g.dotName,
        g.dotPath,
        "SABase",
        cacheMisses,
        tries,
        swaps,
        alg_type,
        tc,
        c2n,
        n2c
    );
    return report;
}


void getSwapCost(
    const std::vector<int>& n2c,
    const int a,
    const int b,
    const int cellA,
    const int cellB,
    const int nCellsSqrt,
    const std::vector<std::vector<int>>& neighbors,
    int& costABefore,
    int& costAAfter,
    int& costBBefore,
    int& costBAfter
)
{
    costABefore = 0;
    costAAfter = 0;
    costBBefore = 0;
    costBAfter = 0;

    if (a != -1)
    {
        for (const int node : neighbors[a])
        {
            costABefore += getManhattanDist(cellA, n2c[node], nCellsSqrt);
            if (cellB == n2c[node])
            {
                costAAfter += getManhattanDist(cellA, cellB, nCellsSqrt);
            }
            else
            {
                costAAfter += getManhattanDist(cellB, n2c[node], nCellsSqrt);
            }
        }
    }

    if (b != -1)
    {
        for (const int node : neighbors[b])
        {
            costBBefore += getManhattanDist(cellB, n2c[node], nCellsSqrt);
            if (cellA == n2c[node])
            {
                costBAfter += getManhattanDist(cellB, cellA, nCellsSqrt);
            }
            else
            {
                costBAfter += getManhattanDist(cellA, n2c[node], nCellsSqrt);
            }
        }
    }
}
