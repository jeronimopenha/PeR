#include <qca/qcaUtil.h>
#include <iostream>
#include <random>
#include <utility>
#include <common/util.h>

QcaReportData::QcaReportData()
    : success(false), allPLaced(false), _time(0), nCellsSqrt(0), wires(0), nNodes(0), tries(0), swaps(0), wrongEdges(0),
      area(0),
      usedAreaPercentage(0), extraLayers(0)
{
}

// Constructor for easy initialization
QcaReportData::QcaReportData(const bool success, const bool allPLaced, const float _time, string dotName,
                             string dotPath, string placer,
                             const int nCellsSqrt, const int wires, const int nNodes, const int tries, const int swaps,
                             const int wrongEdges, const int area, const float usedAreaPercentage,
                             const int extraLayers,
                             vector<int> extraLayersLevels, vector<int> placement,
                             vector<int> n2c, vector<pair<int, int>> edges)
    : success(success),
      allPLaced(allPLaced),
      _time(_time),
      dotName(std::move(dotName)),
      dotPath(std::move(dotPath)),
      placer(std::move(placer)),
      nCellsSqrt(nCellsSqrt),
      wires(wires),
      nNodes(nNodes),
      tries(tries),
      swaps(swaps),
      wrongEdges(wrongEdges),
      area(area),
      usedAreaPercentage(usedAreaPercentage),
      extraLayers(extraLayers),
      extraLayersLevels(std::move(extraLayersLevels)),
      placement(std::move(placement)),
      n2c(std::move(n2c)),
      edges(std::move(edges))
{
}

// Serialize ReportData to a JSON string
string QcaReportData::to_json() const
{
    ostringstream oss;
    oss << "{\n"
        << "  \"success\": " << success << ",\n"
        << "  \"allPlaced\": " << allPLaced << ",\n"
        << "  \"time\": " << _time << ",\n"
        << "  \"dotName\": \"" << dotName << "\",\n"
        << "  \"dotPath\": \"" << dotPath << "\",\n"
        << "  \"placer\": \"" << placer << "\",\n"
        << "  \"wires\": " << wires << ",\n"
        << "  \"nNodes\": " << nNodes << ",\n"
        << "  \"tries\": " << tries << ",\n"
        << "  \"swaps\": " << swaps << ",\n"
        << "  \"wrongEdges\": " << wrongEdges << ",\n"
        << "  \"area\": " << area << ",\n"
        << "  \"usedAreaPercentage\": " << usedAreaPercentage << ",\n"
        << "  \"extraLayers\": " << extraLayers << ",\n"

        // Serialize vector<int> extraLayersLevels
        << "  \"extraLayersLevels\": [";
    for (size_t i = 0; i < extraLayersLevels.size(); ++i)
    {
        oss << extraLayersLevels[i];
        if (i < extraLayersLevels.size() - 1) oss << ", ";
    }
    oss << "],\n";

    oss << "  \"placement\": [";

    // Serialize vector<int> placement
    for (size_t i = 0; i < placement.size(); ++i)
    {
        oss << placement[i];
        if (i < placement.size() - 1) oss << ", ";
    }
    oss << "],\n";

    // Serialize vector<int> n2c
    oss << "  \"n2c\": [";
    for (size_t i = 0; i < n2c.size(); ++i)
    {
        oss << n2c[i];
        if (i < n2c.size() - 1) oss << ", ";
    }
    oss << "]\n";

    oss << "}\n";
    return oss.str();
}

// Returns the relative offset vectors for input directions of a cell at (x, y)
vector<pair<int, int>> qcaGetOutputDirections(const int x, const int y)
{
    vector<pair<int, int>> directions;
    if (y % 2 == 0)
    {
        // Even row: alternate horizontally
        if (x % 2 == 0)
        {
            directions.emplace_back(0, -1); // top
            directions.emplace_back(-1, 0); // left
        }
        else
        {
            directions.emplace_back(-1, 0); // left
            directions.emplace_back(0, 1); // bottom
        }
    }
    else
    {
        // Odd row: alternate horizontally
        if (x % 2 == 0)
        {
            directions.emplace_back(1, 0); // right
            directions.emplace_back(0, -1); // top
        }
        else
        {
            directions.emplace_back(0, 1); // bottom
            directions.emplace_back(1, 0); // right
        }
    }
    return directions;
}

// Returns the relative offset vectors for output directions of a cell at (x, y)
vector<pair<int, int>> qcaGetInputDirections(const int x, const int y)
{
    vector<pair<int, int>> directions;
    if (y % 2 == 0)
    {
        // Even row: alternate horizontally
        if (x % 2 == 0)
        {
            directions.emplace_back(1, 0); // right
            directions.emplace_back(0, 1); // bottom
        }
        else
        {
            directions.emplace_back(0, -1); // top
            directions.emplace_back(1, 0); // right
        }
    }
    else
    {
        // Odd row: alternate horizontally
        if (x % 2 == 0)
        {
            directions.emplace_back(-1, 0); // left
            directions.emplace_back(0, 1); // bottom
        }
        else
        {
            directions.emplace_back(0, -1); // top
            directions.emplace_back(-1, 0); // left
        }
    }
    return directions;
}


bool qcaIsInvalidCell(const int x, const int y, const int nCellsSqrt)
{
    const bool outOfBounds = (x < 0 || x >= nCellsSqrt || y < 0 || y >= nCellsSqrt);
    return outOfBounds;
}

void qcaExportUSEToDot(const string& filename, const vector<int>& n2c, const vector<pair<int, int>>& edges,
                       int nCellsSqrt)
{
    ofstream file(filename);
    if (!file)
    {
        cerr << "Error!" << endl;
        return;
    }

    vector<int> cells(nCellsSqrt * nCellsSqrt, -1);

    for (int i = 0; i < n2c.size(); i++)
        if (n2c[i] > -1)
            cells[n2c[i]] = i;


    // write the dot header
    file << "digraph layout{" << endl;
    file << "rankdir=TB; \n" << endl;
    file << "splines=ortho; \n" << endl;
    file << "node [style=filled shape=square fixedsize=true width=0.6];" << endl;

    for (int i = 0; i < cells.size(); i++)
    {
        if (cells[i] == -1)
        {
            file << i << "[label=\"\", fontsize=8, fillcolor=\"#ffffff\"];" << endl;
        }
        else
        {
            file << i << "[label=\"" << cells[i] << "\", fontsize=8, fillcolor=\"#a9ccde\"];" << endl;
        }
    }
    file << "edge [constraint=false, style=vis];" << endl;
    //normal edges
    int nCells = static_cast<int>(pow(nCellsSqrt, 2));
    for (int cell = 0; cell < nCells; cell++)
    {
        int x0 = getX(cell, nCellsSqrt);
        int y0 = getY(cell, nCellsSqrt);
        vector<pair<int, int>> direction = qcaGetOutputDirections(x0, y0);
        int x1 = x0 + direction[0].first;
        int y1 = y0 + direction[0].second;
        int x2 = x0 + direction[1].first;
        int y2 = y0 + direction[1].second;

        int cell1 = getCellIndex(x1, y1, nCellsSqrt);
        int cell2 = getCellIndex(x2, y2, nCellsSqrt);

        if (!qcaIsInvalidCell(x1, y1, nCellsSqrt))
        {
            file << cell << " -> " << cell1;
            file << " [color=\"#cccccc\"];" << endl;
            /*if (cells[cell1] == -1)
                file << " [color=\"#cccccc\"];" << endl;
            else
                file << ";" << endl;*/
        }
        if (!qcaIsInvalidCell(x2, y2, nCellsSqrt))
        {
            file << cell << " -> " << cell2;
            file << " [color=\"#cccccc\"];" << endl;
            /*if (cells[cell2] == -1)
                file << " [color=\"#cccccc\"];" << endl;
            else
                file << ";" << endl;*/
        }
    }

    for (const auto& [u, v] : edges)
    {
        if (u >= 0 && v >= 0 && u < n2c.size() && v < n2c.size())
        {
            const int cellU = n2c[u];
            const int cellV = n2c[v];
            if (cellU != -1 && cellV != -1)
                file << cellU << " -> " << cellV << ";" << endl;
        }
    }

    file << "edge [constraint=true, style=invis];" << endl;
    //structural edges
    for (int j = 0; j < nCellsSqrt; j++)
    {
        for (int i = 0; i < nCellsSqrt; i++)
        {
            int c = j + i * nCellsSqrt;
            if (i == nCellsSqrt - 1)
            {
                file << c << ";" << endl;
            }
            else
            {
                file << c << " -> ";
            }
        }
    }

    for (int i = 0; i < nCellsSqrt; i++)
    {
        file << "rank = same { ";
        for (int j = 0; j < nCellsSqrt; j++)
        {
            int c = i * nCellsSqrt + j;
            if (j == nCellsSqrt - 1)
            {
                file << c << ";";
            }
            else
            {
                file << c << " -> ";
            }
        }
        file << "};" << endl;
    }


    // write the dot footer
    file << "}" << endl;
    file.close();
}

AreaMetrics computeOccupiedAreaMetrics(const int nCellsSqrt, const vector<int>& c2n)
{
    int minRow = numeric_limits<int>::max();
    int maxRow = -1;
    int minCol = numeric_limits<int>::max();
    int maxCol = -1;
    int totalOccupied = 0;

    for (int idx = 0; idx < c2n.size(); ++idx)
        if (c2n[idx] != -1)
        {
            int row = idx / nCellsSqrt;
            int col = idx % nCellsSqrt;

            minRow = min(minRow, row);
            maxRow = max(maxRow, row);
            minCol = min(minCol, col);
            maxCol = max(maxCol, col);

            totalOccupied++;
        }


    AreaMetrics result{};
    result.minRow = minRow;
    result.maxRow = maxRow;
    result.minCol = minCol;
    result.maxCol = maxCol;

    const int height = maxRow - minRow + 1;
    const int width = maxCol - minCol + 1;
    result.totalCells = height * width;
    result.occupiedCells = totalOccupied;
    result.utilization = result.totalCells > 0
                             ? static_cast<float>(totalOccupied) / static_cast<float>(result.totalCells)
                             : 0.0f;

    return result;
}

void qcaWriteJson(const string& basePath,
                  const string& reportPath,
                  const string& algPath,
                  const string& fileName,
                  const int extraLayers,
                  const QcaReportData& data)
{
    string finalPath = basePath + reportPath + algPath + "/json/";
    string jsonFile = finalPath + fileName + ".json";

    createDir(finalPath);

    ofstream file(jsonFile);

    if (file.is_open())
    {
        file << data.to_json();; // Write JSON string to file
        file.close();
    }
    else
        cerr << "Error opening file for writing: " << fileName << ".json" << endl;
}

bool allPLaced(const vector<int>& n2c)
{
    bool placed = true;
    for (const auto cell : n2c)
        if (cell == -1)
        {
            placed = false;
            break;
        }

    return placed;
}
