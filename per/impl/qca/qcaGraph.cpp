#include <algorithm>
#include <queue>
#include <qca/qcaGraph.h>


QCAGraph::QCAGraph(const string &dotPath, const string &dotName): Graph(dotPath, dotName) {
}

// find the level of each node from the input nodes
unordered_map<int, int> QCAGraph::computeLevels(const vector<int> &inputNodes, const vector<pair<int, int> > &edges) {
    unordered_map<int, vector<int> > graph;
    unordered_map<int, int> inDegree;
    unordered_map<int, int> level;

    // build the graph and initialize the auxiliar variables
    for (auto [fst, snd]: edges) {
        graph[fst].push_back(snd);
        inDegree[snd]++;
        level[fst] = 0;
        level[snd] = 0;
    }

    // queue for the topological order processing
    queue<int> q;
    for (int node: inputNodes) {
        q.push(node);
        level[node] = 0;
    }

    while (!q.empty()) {
        int fst = q.front();
        q.pop();
        for (int v: graph[fst]) {
            level[v] = max(level[v], level[fst] + 1);
            if (--inDegree[v] == 0)
                q.push(v);
        }
    }

    return level;
}

// balance the graph inserting delay nodes
void QCAGraph::balanceGraph() {
    // Step 1: Compute level (depth) of each node starting from the inputs
    unordered_map<int, int> level = computeLevels(inputNodes, gEdges);

    // Prepare new edge list and ID counter for new dummy nodes
    vector<pair<int, int> > newEdges;
    int nextNodeId = *max_element(gNodes.begin(), gNodes.end()) + 1;

    for (const auto &[fst, snd]: gEdges) {
        int lu = level[fst];
        int lv = level[snd];

        if (lv == lu + 1) {
            // Edge is already balanced (no delay needed)
            newEdges.emplace_back(fst, snd);
        } else if (lv > lu + 1) {
            // Path from u to v is too short, insert dummy nodes
            int last = fst;
            for (int i = 0; i < lv - lu - 1; ++i) {
                int dummy = nextNodeId++;
                gNodes.push_back(dummy);
                newEdges.emplace_back(last, dummy);
                last = dummy;
            }
            newEdges.emplace_back(last, snd);
        } else {
            // This should not happen in a valid DAG (Backward edge?)
            cerr << "⚠️ Warning: inconsistent level between " << fst << " and " << snd << endl;
        }
    }

    // Replace the old edge list with the new balanced one
    gEdges = newEdges;
}
