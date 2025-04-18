#include <common/graph.h>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

using namespace std;

Graph::Graph(const string &dotPath, const string &dotName, const bool str) {
    this->dotPath = dotPath;
    this->dotName = dotName;

    if (str)
        readGraphDataStr();
    else
        readEdgesNodes();
    updateG();
}

void Graph::updateG() {
    readAdjList();
    readSuccPred();
    readIONodes();
}

void Graph::readEdgesNodes() {
    unordered_set<long> nodes;

    ifstream dotFile(dotPath);
    string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << dotPath << endl;
        return;
    }

    while (getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != string::npos) {
            string toNode;
            string fromNode;

            istringstream iss(line);
            string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            toNode.erase(remove(toNode.begin(), toNode.end(), '\"'), toNode.end());
            fromNode.erase(remove(fromNode.begin(), fromNode.end(), '\"'), fromNode.end());

            long fromN = stol(fromNode);
            long toN = stol(toNode);

            // Add the edge to the adjacency list

            nodes.insert(fromN);
            nodes.insert(toN);
            gEdges.emplace_back(fromN, toN);
        }
    }
    dotFile.close();
    nEdges = static_cast<int>(gEdges.size());
    nNodes = static_cast<int>(nodes.size());

    //input and output nodes
    for (long i = 0; i < nNodes; i++)
        gNodes.push_back(i);
}

void Graph::readAdjList() {
    adjList.clear();
    for (auto [fst, snd]: gEdges) {
        adjList[fst].push_back(snd);
    }
}

void Graph::readIONodes() {
    //input and output nodes
    outputNodes.clear();
    inputNodes.clear();
    otherNodes.clear();
    for (long i = 0; i < nNodes; i++) {
        if (nSuccV[i] == 0) {
            outputNodes.push_back(i);
            continue;
        }
        if (nPredV[i] == 0) {
            inputNodes.push_back(i);
            continue;
        }
        otherNodes.push_back(i);
    }
}

void Graph::readSuccPred() {
    //2 - find the successors and the predecessors
    //and find how many succ and pred each node have

    //edgesIdx
    //successors
    //predecessors
    nSuccV = vector<long>(nNodes, 0);
    nPredV = vector<long>(nNodes, 0);
    successors = vector(nNodes, vector(nNodes, false));
    predecessors = vector(nNodes, vector(nNodes, false));

    for (const auto &[fst, snd]: gEdges) {
        const long fromN = fst;
        const long toN = snd;

        successors[fromN][toN] = true;
        predecessors[toN][fromN] = true;

        nSuccV[fromN] += 1;
        nPredV[toN] += 1;
    }
}

vector<pair<long, long> > Graph::getEdgesDepthFirst() {
    // Copy input nodes and shuffle if needed
    vector<long> inputList = inputNodes;

    randomVector(inputList);

    vector stack(inputList); // Initialize stack with input_list
    vector<pair<long, long> > edges;
    vector visited(nNodes, false);

    while (!stack.empty()) {
        long n = stack.back();
        stack.pop_back();

        if (visited[n])
            continue;

        visited[n] = true;
        // Process all neighbors
        for (long i = 0; i < nNodes; i++) {
            if (successors[n][i]) {
                if (!visited[i]) {
                    stack.push_back(i);
                    edges.emplace_back(n, i);
                }
            }
        }
    }

    return edges;
}


vector<pair<long, long> > Graph::getEdgesZigzag(
    vector<pair<long, long> > &convergence,
    vector<tuple<long, long, string> > *edgeTypes) {
    vector<pair<long, string> > outputList;

    for (const auto &node: outputNodes)
        outputList.emplace_back(node, "IN");

    randomVector(outputList);

    vector stack(outputList.begin(), outputList.end());
    vector<pair<long, long> > edges;
    vector visited(nNodes, false);

    // Precompute fan-in and fan-out
    vector<vector<long> > fanIn(nNodes);
    vector<vector<long> > fanOut(nNodes);
    for (long i = 0; i < nNodes; i++) {
        for (long j = 0; j < nNodes; j++) {
            if (successors[i][j]) {
                fanOut[i].push_back(j);
            }
            if (predecessors[i][j]) {
                fanIn[i].push_back(j);
            }
        }
        randomVector(fanOut[i]);
        randomVector(fanIn[i]);
    }


    while (!stack.empty()) {
        auto [fst, snd] = stack.back();
        stack.pop_back();

        const long &a = fst;
        const string &direction = snd;
        visited[a] = true;;

        if (direction == "IN") {
            if (!fanOut[a].empty()) {
                const long b = fanOut[a].back();
                stack.emplace_back(a, "IN");
                stack.insert(stack.end(), fanIn[a].size(), {a, "IN"});
                stack.emplace_back(b, "OUT");

                fanOut[a].pop_back();
                fanIn[b].erase(remove(fanIn[b].begin(), fanIn[b].end(), a), fanIn[b].end());

                if (visited[b])
                    convergence.emplace_back(a, b);

                if (edgeTypes)
                    edgeTypes->emplace_back(a, b, "OUT");
                edges.emplace_back(a, b);
            } else if (!fanIn[a].empty()) {
                const long b = fanIn[a].back();
                stack.emplace_back(a, "IN");
                stack.insert(stack.end(), fanIn[a].size(), {b, "IN"});

                fanIn[a].pop_back();
                fanOut[b].erase(remove(fanOut[b].begin(), fanOut[b].end(), a), fanOut[b].end());

                if (visited[b])
                    convergence.emplace_back(a, b);

                if (edgeTypes)
                    edgeTypes->emplace_back(a, b, "IN");
                edges.emplace_back(a, b);
            }
        } else {
            // direction == "OUT"
            if (!fanIn[a].empty()) {
                long b = fanIn[a].back();
                stack.emplace_back(a, "OUT");
                stack.insert(stack.end(), fanOut[a].size(), {a, "OUT"});
                stack.emplace_back(b, "IN");

                fanIn[a].pop_back();
                fanOut[b].erase(remove(fanOut[b].begin(), fanOut[b].end(), a), fanOut[b].end());

                if (visited[b])
                    convergence.emplace_back(a, b);

                if (edgeTypes)
                    edgeTypes->emplace_back(a, b, "IN");
                edges.emplace_back(a, b);
            } else if (!fanOut[a].empty()) {
                long b = fanOut[a].back();
                stack.emplace_back(a, "OUT");
                stack.insert(stack.end(), fanOut[a].size(), {b, "OUT"});

                fanOut[a].pop_back();
                fanIn[b].erase(remove(fanIn[b].begin(), fanIn[b].end(), a), fanIn[b].end());

                if (visited[b])
                    convergence.emplace_back(a, b);

                if (edgeTypes)
                    edgeTypes->emplace_back(a, b, "OUT");
                edges.emplace_back(a, b);
            }
        }
    }
    edges = clearEdges(edges);

    if (edgeTypes) {
        vector<tuple<long, long, string> > cleaned;
        for (const auto &t: *edgeTypes) {
            const long a = get<0>(t);
            const long b = get<1>(t);
            if (find(edges.begin(), edges.end(), make_pair(a, b)) != edges.end()) {
                cleaned.push_back(t);
            }
        }
        *edgeTypes = cleaned;
    }
    return edges; //clearEdges(edges);
}

vector<pair<long, long> > Graph::clearEdges(const vector<pair<long, long> > &edges) const {
    vector placedNodes(nNodes, false); // Set to track placed nodes
    vector<pair<long, long> > new_edges; // Vector to store filtered edges

    // Add the first node of the first edge to the set
    placedNodes[edges[0].first] = true;

    for (const auto [fst,snd]: edges) {
        // Check if the second node is not in the set or if we don't remove placed edges
        if (!placedNodes[snd]) {
            placedNodes[snd] = true; // Add n2 to the set
            new_edges.emplace_back(fst, snd); // Add the edge to the new list
        }
    }
    return new_edges;
}


void Graph::dfs(const long idx, const vector<vector<long> > &adj, vector<bool> &visited, vector<long> &topo_order) {
    visited[idx] = true;
    for (const long v: adj[idx]) {
        if (!visited[v])
            dfs(v, adj, visited, topo_order);
    }
    topo_order.push_back(idx);
}

void Graph::readGraphDataStr() {
    unordered_set<string> nodesStr;

    ifstream dotFile(dotPath);
    string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << dotPath << endl;
        return;
    }

    //1 - Read edges and get a list of nodes
    while (getline(dotFile, line)) {
        // Look for lines that define edges

        if (line.find("->") != string::npos) {
            string toNode;
            string fromNode;

            istringstream iss(line);
            string word;
            // Get the fromNode
            iss >> fromNode;
            // Ignore the "->" part
            iss >> word;
            // Get the toNode
            iss >> toNode;
            // Remove any trailing characters (like semicolon)
            toNode.erase(remove(toNode.begin(), toNode.end(), ';'), toNode.end());
            toNode.erase(remove(toNode.begin(), toNode.end(), '\"'), toNode.end());
            fromNode.erase(remove(fromNode.begin(), fromNode.end(), '\"'), fromNode.end());
            // Add the edge to the adjacency list

            nodesStr.insert(fromNode);
            nodesStr.insert(toNode);
            gEdgesStr.emplace_back(fromNode, toNode);
            nEdges += 1;
        }
    }
    dotFile.close();
    nEdges = static_cast<long>(gEdges.size());
    nNodes = static_cast<long>(nodesStr.size());

    //2 - Create the dictionary nodesToIdx
    std::unordered_map<std::string, long> nodesToIdx;

    long counter = 0;
    for (const auto &node: nodesStr) {
        nodesToIdx[node] = counter;
        counter++;
    }

    //create the int edges
    for (const auto &[fst, snd]: gEdgesStr) {
        const long fromN = nodesToIdx[fst];
        const long toN = nodesToIdx[snd];
        gEdges.emplace_back(fromN, toN);
    }

    for (long i = 0; i < nNodes; i++)
        gNodes.push_back(i);
}


void Graph::isolateMultiInputOutputs() {
    vector<long> newOutputs;
    vector<pair<long, long> > newEdges;
    long nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;

    for (auto node: outputNodes) {
        const bool hasMoreInput = count(predecessors[node].begin(), predecessors[node].end(), true) > 1;;

        if (hasMoreInput) {
            // Cria dummy de saída
            long dummy = nextId++;
            gNodes.push_back(dummy);
            nNodes++;

            newEdges.emplace_back(node, dummy);
            newOutputs.push_back(dummy);
        } else
            newOutputs.push_back(node);
    }

    // Atualiza lista de outputs
    outputNodes = newOutputs;

    // Adiciona nova aresta ao grafo
    gEdges.insert(gEdges.end(), newEdges.begin(), newEdges.end());

    updateG(); // atualiza estruturas internas
}

/*
* bool hasMultiplePredecessors(int u, const vector<vector<bool>>& predecessors) {
    return std::count(predecessors[u].begin(), predecessors[u].end(), true) > 1;
}
 */
