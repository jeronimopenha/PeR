#include <common/graph.h>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <queue>
#include <stack>

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
    readTypeOfNodes();
    readAsapAlap();
}

void Graph::readEdgesNodes() {
    unordered_set<long> nodes;
    unordered_set<long> declaredNodes;

    ifstream dotFile(dotPath);
    string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << dotPath << endl;
        return;
    }

    gEdges.clear();
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
        } else if (line.find(';') != string::npos && line.find("->") == string::npos) {
            istringstream iss(line);
            string maybeNodeStr;
            iss >> maybeNodeStr;
            maybeNodeStr.erase(remove(maybeNodeStr.begin(), maybeNodeStr.end(), ';'), maybeNodeStr.end());
            maybeNodeStr.erase(remove(maybeNodeStr.begin(), maybeNodeStr.end(), '\"'), maybeNodeStr.end());

            if (!maybeNodeStr.empty()) {
                declaredNodes.insert(stol(maybeNodeStr));
            }
        }
    }
    dotFile.close();

    for (const long &node: declaredNodes) {
        if (nodes.find(node) == nodes.end()) {
            disconnectedNodes.push_back(node);
        }
    }

    //nodes.insert(disconnectedNodes.begin(), disconnectedNodes.end());

    nEdges = static_cast<int>(gEdges.size());
    nNodes = static_cast<int>(nodes.size());

    gNodes.clear();
    gNodes.resize(nNodes);
    iota(gNodes.begin(), gNodes.end(), 0);
}

void Graph::readAdjList() {
    //optimization
    if (succList.size() != nNodes)
        succList.resize(nNodes);
    else
        for (auto &v: succList)
            v.clear();

    if (predList.size() != nNodes)
        predList.resize(nNodes);
    else
        for (auto &v: predList)
            v.clear();
    for (auto [fst, snd]: gEdges) {
        succList[fst].push_back(snd);
        predList[snd].push_back(fst);
    }
}

void Graph::readSuccPred() {
    //2 - find the successors and the predecessors
    //and find how many succ and pred each node have

    //edgesIdx
    //successors
    //predecessors
    // Optimization
    if (nSuccV.size() != nNodes)
        nSuccV.resize(nNodes);
    if (nPredV.size() != nNodes)
        nPredV.resize(nNodes);
    if (successors.size() != nNodes)
        successors.resize(nNodes, vector(nNodes, false));
    if (predecessors.size() != nNodes)
        predecessors.resize(nNodes, vector<bool>(nNodes, false));

    fill(nSuccV.begin(), nSuccV.end(), 0);
    fill(nPredV.begin(), nPredV.end(), 0);

    for (auto &row: successors) {
        if (row.size() != nNodes)
            row.resize(nNodes);
        fill(row.begin(), row.end(), false);
    }

    for (auto &row: predecessors) {
        if (row.size() != nNodes)
            row.resize(nNodes);
        fill(row.begin(), row.end(), false);
    }

    for (const auto &[fst, snd]: gEdges) {
        const long fromN = fst;
        const long toN = snd;

        successors[fromN][toN] = true;
        predecessors[toN][fromN] = true;

        nSuccV[fromN] += 1;
        nPredV[toN] += 1;
    }
}

void Graph::readTypeOfNodes() {
    //input and output nodes
    outputNodes.clear();
    inputNodes.clear();
    innerNodes.clear();

    for (long node = 0; node < nNodes; node++) {
        if (nSuccV[node] == 0) {
            outputNodes.push_back(node);
            continue;
        }
        if (nPredV[node] == 0) {
            inputNodes.push_back(node);
            continue;
        }
        innerNodes.push_back(node);
    }
}

void Graph::readAsapAlap() {
    // Optimization
    if (asap.size() != nNodes) {
        asap.resize(nNodes);
        alap.resize(nNodes);
        slack.resize(nNodes);
    }

    vector<long> pendingPred(nNodes, 0);
    vector<long> pendingSucc(nNodes, 0);

    fill(asap.begin(), asap.end(), 1);

    for (long from_n = 0; from_n < nNodes; ++from_n) {
        for (const long to_n: succList[from_n]) {
            pendingPred[to_n]++;
            pendingSucc[from_n]++;
        }
    }

    //-----ASAP-----
    queue<long> q;
    // Enqueue the input nodes
    for (const auto &node: inputNodes) {
        q.push(node);
    }

    while (!q.empty()) {
        const long node = q.front();
        q.pop();

        for (const long succ: succList[node]) {
            asap[succ] = max(asap[succ], asap[node] + 1);
            pendingPred[succ]--;
            if (pendingPred[succ] == 0) {
                q.push(succ);
            }
        }
    }

    //-----ALAP-----
    const long maxLevel = *max_element(asap.begin(), asap.end());
    fill(alap.begin(), alap.end(), maxLevel);

    // Enqueue the output nodes
    for (const auto &node: outputNodes) {
        q.push(node);
    }

    // ALAP traversal (backward)
    while (!q.empty()) {
        const long node = q.front();
        q.pop();

        for (const long pred: predList[node]) {
            alap[pred] = min(alap[pred], alap[node] - 1);
            pendingSucc[pred]--;
            if (pendingSucc[pred] == 0) {
                q.push(pred);
            }
        }
    }

    for (auto i = 0; i < nNodes; i++) {
        slack[i] = alap[i] - asap[i];
    }
}

vector<pair<long, long> > Graph::getEdgesDepthFirstCritical(const bool criticalPriority) {
    vector<pair<long, long> > edges;

    vector<long> visited;
    if (visited.size() != nNodes) {
        visited.resize(nNodes);
    }
    fill(visited.begin(), visited.end(), false);

    stack<pair<long, long> > s;
    while (!s.empty()) {
        s.pop();
    }

    //sort outputs by slack
    vector<long> outputList = outputNodes;
    outputList.insert(outputList.end(), disconnectedNodes.begin(), disconnectedNodes.end());
    randomVector(outputList);
    sort(outputList.begin(), outputList.end(), [&](const long a, const long b) {
        const long slack_a = slack[a];
        const long slack_b = slack[b];
        return slack_a > slack_b;
    });

    //insert face edge to output nodes
    /*for (const auto node: outputList) {
        s.push(node);
        edges.emplace_back(-1, node);
    }*/

    s.emplace(-1, -1);

    vector<long> neigh;

    while (!s.empty()) {
        const long last = s.top().first;
        const long node = s.top().second;

        s.pop();

        neigh.clear();

        if (node != -1) {
            if (visited[node])
                continue;
            visited[node] = true;
        }
        // Process all neighbors

        if (node != -1) {
            edges.emplace_back(last, node);
        }

        if (node == -1) {
            neigh.insert(neigh.end(), outputList.begin(), outputList.end());
        } else {
            for (const auto pred: predList[node])
                neigh.push_back(pred);
        }

        sort(neigh.begin(), neigh.end(), [&](const long a, const long b) {
            const long slack_a = slack[a];
            const long slack_b = slack[b];
            return slack_a > slack_b;
        });

        for (const auto pred: neigh) {
            if (!visited[pred]) {
                s.emplace(node, pred);
                //edges.emplace_back(node, pred);
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
    sort(outputList.begin(), outputList.end(), [&](const pair<long, string> &a, const pair<long, string> &b) {
        const long slack_a = slack[a.first];
        const long slack_b = slack[b.first];
        return slack_a > slack_b;
    });

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
        sort(fanOut[i].begin(), fanOut[i].end(), [&](const long a, const long b) {
            const long slack_a = slack[a];
            const long slack_b = slack[b];
            return slack_a > slack_b;
        });
        randomVector(fanIn[i]);
        sort(fanIn[i].begin(), fanIn[i].end(), [&](const long a, const long b) {
            const long slack_a = slack[a];
            const long slack_b = slack[b];
            return slack_a > slack_b;
        });
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
    edges = clearZigZagEdges(edges);

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

vector<pair<long, long> > Graph::clearZigZagEdges(const vector<pair<long, long> > &edges) const {
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
    unordered_set<string> declaredNodesStr;
    unordered_set<string> declaredButDisconnectedStr;

    ifstream dotFile(dotPath);
    string line;

    // If  the opening has an error
    if (!dotFile.is_open()) {
        cerr << "Error opening file: " << dotPath << endl;
        return;
    }

    gEdgesStr.clear();
    //1 - Read edges and get a list of nodes
    while (getline(dotFile, line)) {
        // Look for lines that define edges
        if (line.find("->") != string::npos) {
            string fromNode;
            string toNode;

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
        } else if (line.find(';') != string::npos && line.find("->") == string::npos) {
            istringstream iss(line);
            string maybeNodeStr;
            iss >> maybeNodeStr;
            maybeNodeStr.erase(remove(maybeNodeStr.begin(), maybeNodeStr.end(), ';'), maybeNodeStr.end());
            maybeNodeStr.erase(remove(maybeNodeStr.begin(), maybeNodeStr.end(), '\"'), maybeNodeStr.end());

            if (!maybeNodeStr.empty()) {
                declaredNodesStr.insert(maybeNodeStr);
            }
        }
    }
    dotFile.close();

    for (const auto &node: declaredNodesStr) {
        if (nodesStr.find(node) == nodesStr.end()) {
            declaredButDisconnectedStr.insert(node);
        }
    }

    nodesStr.insert(declaredButDisconnectedStr.begin(), declaredButDisconnectedStr.end());

    nEdges = static_cast<long>(gEdges.size());
    nNodes = static_cast<long>(nodesStr.size());

    //2 - Create the dictionary nodesToIdx
    std::unordered_map<std::string, long> nodesToIdx;

    long counter = 0;
    for (const auto &node: nodesStr) {
        nodesToIdx[node] = counter;
        counter++;
    }

    gEdges.clear();

    //create the int edges
    for (const auto &[fst, snd]: gEdgesStr) {
        const long fromN = nodesToIdx[fst];
        const long toN = nodesToIdx[snd];
        gEdges.emplace_back(fromN, toN);
    }

    gNodes.clear();
    gNodes.resize(nNodes);
    iota(gNodes.begin(), gNodes.end(), 0);

    disconnectedNodes.clear();
    for (const string &nodeStr: declaredButDisconnectedStr) {
        const long nodeIdx = nodesToIdx[nodeStr];
        disconnectedNodes.emplace_back(nodeIdx);
    }
}


void Graph::isolateMultiInputOutputs() {
    vector<long> newOutputs;
    vector<pair<long, long> > newEdges;
    long nextId = *max_element(gNodes.begin(), gNodes.end()) + 1;

    for (auto node: outputNodes) {
        const bool hasMoreInput = count(predecessors[node].begin(), predecessors[node].end(), true) > 1;

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

void Graph::saveToDot(const string &filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error!" << endl;
        return;
    }

    // write the dot header
    file << "digraph G {" << endl;

    // write the edges
    for (const auto &[fst, snd]: gEdges)
        file << "    " << fst << " -> " << snd << ";" << endl;

    for (const long node: disconnectedNodes)
        file << "    " << node << ";" << endl;

    // write the dot footer
    file << "}" << endl;

    file.close();
    cout << "File " << filename << " saved!" << endl;
}
