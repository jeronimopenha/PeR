//
// Created by jeronimo on 01/11/24.
//
#include "graph.h"

Graph::Graph(const std::string &dotPath, const std::string &dotName) {
    this->dotPath = dotPath;
    this->dotName = dotName;
    getGraphData();
}

void Graph::getGraphData() {
    std::ifstream dotFile(this->dotPath);
    std::string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        std::cerr << "Error opening file: " << dotPath << std::endl;
        return;
    }

    std::unordered_set<std::string> nodesStr;
    //succ and edgesStr
    while (std::getline(dotFile, line)) {
        // Look for lines that define edges
        if (line.find("->") != std::string::npos) {
            std::string toNode;
            std::string fromNode;
            n_edges++;
            std::istringstream iss(line);
            std::string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(std::remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            // Add the edge to the adjacency list
            succ[fromNode].insert(toNode);
            edgesStr.emplace_back(fromNode, toNode);
            nodesStr.insert(fromNode);
            nodesStr.insert(toNode);
        }
    }
    dotFile.close();

    //pred
    for (const auto &[fst, snd]: succ) {
        for (const auto &neighbor: snd) {
            pred[neighbor].insert(fst);
        }
    }

    //nodestoidx and idxtonodes
    //inputNodes str and idx
    //outputNodes str and idx
    nNodes = 0;
    for (const auto &node: nodesStr) {
        this->nodesStr.push_back(node);
        nodesToIdx[node] = nNodes;
        idxToNodes[nNodes] = node;

        if (auto it = succ.find(node); it == succ.end()) {
            succ[node] = std::unordered_set<std::string>();
            outputNodesStr.push_back(node);
            outputNodesIdx.push_back(nodesToIdx[node]);
        }
        if (auto it = pred.find(node); it == pred.end()) {
            pred[node] = std::unordered_set<std::string>();
            inputNodesStr.push_back(node);
            inputNodesIdx.push_back(nodesToIdx[node]);
        }
        nNodes++;
    }

    //edgesIdx
    for (const auto &[fst, snd]: edgesStr) {
        edgesIdx.emplace_back(nodesToIdx[fst], nodesToIdx[snd]);
    }

    int totalInOut = inputNodesIdx.size() + outputNodesIdx.size();
    int nBaseNodes = nodesStr.size() - totalInOut;
    int nCellsBaseSqrt = ceil(sqrt(nBaseNodes));
    int nBorderCells = nCellsBaseSqrt * 4;
    while (totalInOut > nBorderCells) {
        nCellsBaseSqrt += 2;
        nBorderCells = nCellsBaseSqrt * 4;
    }
    int nCellsBase = static_cast<int>(pow(nCellsBaseSqrt, 2));
    int totalCells = nCellsBase + nBorderCells;
    nCellsSqrt = ceil(sqrt(totalCells));
    nCells = static_cast<int>(pow(nCellsSqrt, 2));
}

std::vector<std::pair<int, int> >
Graph::getEdgesIdx(const std::vector<std::pair<std::string, std::string> > &edgesStr) {
    std::vector<std::pair<int, int> > edgesIdx;
    for (const auto &[fst,snd]: edgesStr) {
        edgesIdx.emplace_back(nodesToIdx[fst], nodesToIdx[snd]);
    }
    return edgesIdx;
}

std::vector<int> Graph::getNodesIdx(const std::vector<std::string> &nodesStr) {
    std::vector<int> nodesIdx;
    for (const auto &node: nodesStr) {
        nodesIdx.push_back(nodesToIdx[node]);
    }
    return nodesIdx;
}

std::vector<std::vector<std::vector<int> > > Graph::getMeshDistances() {
    int max_dist = (nCellsSqrt - 1) * 2;
    std::vector<std::vector<std::vector<int> > > distance_table_raw(max_dist);

    for (int i = 0; i < nCellsSqrt; ++i) {
        for (int j = 0; j < nCellsSqrt; ++j) {
            if (i == 0 && j == 0) continue; // Skip t
            const int dist = i + j;

            // Lambda to check if a coordinate pair is already in a list
            auto contains = [](const std::vector<std::vector<int> > &vec, const std::vector<int> &pair) {
                return std::find(vec.begin(), vec.end(), pair) != vec.end();
            };

            // Add unique coordinates to the distance table
            if (!contains(distance_table_raw[dist - 1], {i, j})) {
                distance_table_raw[dist - 1].push_back({i, j});
            }
            if (!contains(distance_table_raw[dist - 1], {i, -j})) {
                distance_table_raw[dist - 1].push_back({i, -j});
            }
            if (!contains(distance_table_raw[dist - 1], {-i, -j})) {
                distance_table_raw[dist - 1].push_back({-i, -j});
            }
            if (!contains(distance_table_raw[dist - 1], {-i, j})) {
                distance_table_raw[dist - 1].push_back({-i, j});
            }
        }
    }
    // Shuffle the distance table if make_shuffle is true

    auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    for (auto &d: distance_table_raw) {
        std::shuffle(d.begin(), d.end(), rng);
    }

    return distance_table_raw;
}

std::string Graph::funcKey(const std::string &val1, const std::string &val2) {
    return val1 + " " + val2;
}

std::vector<std::string> Graph::funcUnkey(const std::string &text) {
    std::vector<std::string> result;
    std::istringstream stream(text);
    std::string word;

    while (stream >> word) {
        result.push_back(word);
    }
    return result;
}

std::vector<std::pair<std::string, std::string> > Graph::getEdgesDepthFirst() {
    // Copy input nodes and shuffle if needed
    std::vector<std::string> input_list = inputNodesStr;

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(input_list.begin(), input_list.end(), g);

    std::vector<std::string> stack(input_list); // Initialize stack with input_list
    std::vector<std::pair<std::string, std::string> > edges;
    std::unordered_set<std::string> visited;

    while (!stack.empty()) {
        std::string n = stack.back();
        stack.pop_back();

        if (visited.find(n) != visited.end()) {
            continue;
        }
        visited.insert(n);


        // Process all neighbors without priority
        for (const auto &neigh: succ[n]) {
            if (visited.find(neigh) == visited.end()) {
                stack.push_back(neigh);
                edges.emplace_back(n, neigh);
            }
        }
    }

    return edges;
}
