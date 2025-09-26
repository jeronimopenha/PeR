#include <fpga/fpgaPar.h>
#include <fpga/fpgaGraph.h>
#include <common/util.h>

using namespace std;

/**
 * Class contructor
 * @param dotPath
 * @param dotName
 */
FPGAGraph::FPGAGraph(const string &dotPath, const string &dotName) : Graph(dotPath, dotName) {
    readNeighbors();
    calcMatrix();
    //clbNodes = otherNodes;
    updateG();
}


/**
 * Find all neighbors of each node
 */
void FPGAGraph::readNeighbors() {
    //neighbors vector
    neighbors.clear();
    neighbors.resize(nNodes);
    for (size_t i = 0; i < successors.size(); ++i) {
        for (size_t j = 0; j < successors[i].size(); ++j) {
            if (successors[i][j] || predecessors[i][j])
                neighbors[i].push_back(static_cast<long>(j));
        }
    }
}


/**
 * Calculates the minimal square matrix to place all nodes and I/Os
 */
void FPGAGraph::calcMatrix() {
    const long totalInOut = static_cast<long>(inputNodes.size() + outputNodes.size()); // + disconnectedNodes.size());
    const long nBaseNodes = nNodes - totalInOut;
    long nCellsBaseSqrt = ceil(sqrt(nBaseNodes));
    long nBorderCells = nCellsBaseSqrt * 4;
    while (totalInOut > nBorderCells * IO_NUMBER) {
        nCellsBaseSqrt += 1;
        nBorderCells = nCellsBaseSqrt * 4;
    }
    const long nCellsBase = nCellsBaseSqrt * nCellsBaseSqrt; //static_cast<long>(pow(nCellsBaseSqrt, 2));
    const long totalCells = nCellsBase + nBorderCells;
    nCellsSqrt = ceil(sqrt(totalCells));
    nCells = static_cast<long>(pow(nCellsSqrt, 2));
}

/**
 * Defines which cells are I/O cells to simplify the code execution
 * @return
 */
vector<long> FPGAGraph::getInOutPos() const {
    vector<long> possibleInOut;

    // Append positions in the first range
    for (long i = 1; i < nCellsSqrt - 1; ++i)
        for (long j = 0; j < IO_NUMBER; j++)
            possibleInOut.push_back(i);

    // Append positions in the second range
    for (long i = 1; i < nCellsSqrt - 1; ++i)
        for (long j = 0; j < IO_NUMBER; j++)
            possibleInOut.push_back(i + nCells - nCellsSqrt);

    // Append positions in the third range
    for (long i = nCellsSqrt; i < nCells - nCellsSqrt; i += nCellsSqrt)
        for (long j = 0; j < IO_NUMBER; j++)
            possibleInOut.push_back(i);

    // Append positions in the fourth range
    for (long i = nCellsSqrt * 2 - 1; i < nCells - 1; i += nCellsSqrt)
        for (long j = 0; j < IO_NUMBER; j++)
            possibleInOut.push_back(i);

    return possibleInOut;
}

/**
 * Defines which cells are CLB cells to simplify the code execution
 * @return
 */
vector<long> FPGAGraph::getClbPos() const {
    vector<long> pos;
    for (long i = 1; i < nCellsSqrt - 1; i++) {
        const long start = i * nCellsSqrt + 1;
        const long end = (i + 1) * nCellsSqrt - 1;
        for (long j = start; j < end; j++)
            pos.push_back(j);
    }
    return pos;
}

/**
 * Get annotations for edges for the YOTO algorithm execution
 * @param edges
 * @param convergences
 * @return
 */
unordered_map<string, vector<pair<long, long> > > FPGAGraph::fpgaGetGraphAnnotations(
    const vector<pair<long, long> > &edges,
    const vector<pair<long, long> > &convergences
) {
    unordered_map<string, vector<pair<long, long> > > annotations;
    vector<long> placed;
    // Initialization of the dictionary
    placed.push_back(edges[0].first);
    for (const auto &[fst, snd]: edges) {
        string key = funcKey(to_string(fst), to_string(snd));
        annotations[key] = {};
        placed.push_back(snd);

        //This code tries to make yott guide the neighbot IO nodes to be near the border
        //It is an improvement to the yott code
        bool ioNode = false;
        for (const auto &node: neighbors[snd]) {
            const bool tmp = find(inputNodes.begin(), inputNodes.end(), node) != inputNodes.end() ||
                             find(outputNodes.begin(), outputNodes.end(), node) != outputNodes.end();
            if (tmp) {
                const bool tmp2 = find(placed.begin(), placed.end(), node) != placed.end();
                if (!tmp2)
                    ioNode = true;
            }
        }
        if (ioNode)
            annotations[key].emplace_back(-1, -1);
    }

    for (const auto &[fst,snd]: convergences) {
        const long elem_cycle_begin = fst;
        const long elem_cycle_end = snd;
        list<string> walk_key;
        bool found_start = false;
        long count = 0;
        long value1 = -1;

        for (auto it = edges.rbegin(); it != edges.rend(); ++it) {
            const long a = it->first;
            const long b = it->second;

            if (elem_cycle_begin == b && !found_start) {
                value1 = a;
                string key = funcKey(to_string(value1), to_string(elem_cycle_begin));
                walk_key.push_front(key);
                annotations[key].emplace_back(elem_cycle_end, count);
                count++;
                found_start = true;
            } else if (found_start && (value1 == b || elem_cycle_end == a)) {
                const long value2 = b;
                value1 = a;
                string key = funcKey(to_string(value1), to_string(value2));

                if (value1 != elem_cycle_end && value2 != elem_cycle_end) {
                    walk_key.push_front(key);
                    annotations[key].emplace_back(elem_cycle_end, count);
                    count++;
                } else {
                    // Go back and update values
                    const long half_count = count / 2;
                    auto it2 = walk_key.begin();
                    for (long k = 0; k < half_count; ++k, ++it2) {
                        auto &dic_actual = annotations[*it2];
                        for (auto &[fst,snd]: dic_actual) {
                            if (fst == elem_cycle_end)
                                snd = k + 1;
                        }
                    }
                    break; // Move to the next element in convergences
                }
            }
        }
    }

    return annotations;
}

/**
 * This function returns the offsets to find I/O cells to be placed
 * @return
 */
vector<vector<long> > FPGAGraph::generateIoOffsets() {
    const long nCells = nCellsSqrt;
    vector<long> negatives;
    vector<long> positives;
    vector<vector<long> > result(2);

    result[0].push_back(0);
    for (long i = 1; i < nCells; ++i) {
        result[0].push_back(i);
        result[1].push_back(-i);
    }
    return result;
}

/**
 * Finds wich border will be searchde first and on
 * @param line
 * @param column
 * @return
 */
vector<BorderInfo> FPGAGraph::getIoBordersSequence(long line, long column) {
    const long nRows = nCellsSqrt;
    const long nCols = nCellsSqrt;


    vector<BorderInfo> borders = {
        {abs(line - 0), 0, {column, 0}}, // top
        {abs(column - 0), 2, {0, line}}, // left
        {abs(line - (nRows - 1)), 1, {column, nRows - 1}}, // bottom
        {abs(column - (nCols - 1)), 3, {nCols - 1, line}} // right
    };

    sort(borders.begin(), borders.end(), [](const auto &a, const auto &b) {
        bool comp = a.distance<b.distance;
        return (comp);
    });


    return borders;
}
