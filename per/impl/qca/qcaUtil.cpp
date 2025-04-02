#include <qca/qcaUtil.h>
#include <iostream>
#include <random>
#include <common/util.h>

QcaReportData::QcaReportData()
    : _time(0), cacheMisses(0), tries(0), swaps(0), totalCost(0)
{
}

// Constructor for easy initialization
QcaReportData::QcaReportData(float _time, const string& dot_name, const string& dot_path,
                             const string& placer, int cacheMisses, int tries, int swaps,
                             const string& edges_algorithm,
                             int total_cost, const vector<int>& placement, const vector<int>& n2c)
    : _time(_time), dotName(dot_name), dotPath(dot_path), placer(placer), cacheMisses(cacheMisses), tries(tries),
      swaps(swaps), edgesAlgorithm(edges_algorithm), totalCost(total_cost), placement(placement), n2c(n2c)
{
}

// Serialize ReportData to a JSON string
string QcaReportData::to_json() const
{
    ostringstream oss;
    oss << "{\n"
        << "  \"time\": " << _time << ",\n"
        << "  \"dotName\": \"" << dotName << "\",\n"
        << "  \"dotPath\": \"" << dotPath << "\",\n"
        << "  \"placer\": \"" << placer << "\",\n"
        << "  \"cacheMisses\": " << cacheMisses << ",\n"
        << "  \"tries\": " << tries << ",\n"
        << "  \"swaps\": " << swaps << ",\n"
        << "  \"edgesAlgorithm\": \"" << edgesAlgorithm << "\",\n"
        << "  \"totalCost\": " << totalCost << ",\n"
        << "  \"placement\": [";

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

void qcaExportUSEToDot(const string& filename, const vector<int>& n2c, int nCellsSqrt)
{
    ofstream file(filename);
    if (!file)
    {
        cerr << "Error!" << endl;
        return;
    }

    vector<int> cells(nCellsSqrt * nCellsSqrt, -1);

    for (int i = 0; i < n2c.size(); i++)
    {
        if (n2c[i] > -1)
            cells[n2c[i]] = i;
    }

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
            if (cells[cell1] == -1)
                file << " [color=\"#cccccc\"];" << endl;
            else
                file << ";" << endl;
        }
        if (!qcaIsInvalidCell(x2, y2, nCellsSqrt))
        {
            file << cell << " -> " << cell2;
            if (cells[cell2] == -1)
                file << " [color=\"#cccccc\"];" << endl;
            else
                file << ";" << endl;
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
