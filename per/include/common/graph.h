#ifndef GRAPH_H
#define GRAPH_H

#include <common/util.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>


using namespace std;

class Graph
{
public:
    string dotPath;
    string dotName;

    int nEdges = 0;
    int nNodes = 0;

    //Adjacency for successors
    vector<vector<bool>> successors;
    //Adjacency for predecessors
    vector<vector<bool>> predecessors;
    //Edges list
    vector<pair<int, int>> gEdges;
    //Edges list
    vector<pair<string, string>> gEdgesStr;
    //nodes List
    vector<int> gNodes;

    //adjacency list
    unordered_map<int, vector<int>> adjList;

    vector<int> nSuccV;
    vector<int> nPredV;

    vector<int> inputNodes;
    vector<int> outputNodes;
    vector<int> otherNodes;


    Graph(const string& dotPath, const string& dotName, bool str = false);

    void readGraphDataStr();

    void updateG();

    void readEdgesNodes();

    void readAdjList();

    void readIONodes();

    void readSuccPred();

    vector<pair<int, int>> getEdgesDepthFirst();

    vector<pair<int, int>> getEdgesZigzag(
        vector<pair<int, int>>& convergence,
        vector<tuple<int, int, string>>* edgeTypes = nullptr);

    vector<pair<int, int>> clearEdges(const vector<pair<int, int>>& edges) const;

    void dfs(int idx, const vector<vector<int>>& adj, vector<bool>& visited, vector<int>& topo_order);
};
#endif
