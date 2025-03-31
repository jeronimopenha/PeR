#ifndef GRAPH_H
#define GRAPH_H
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <common/util.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>


using namespace std;

class Graph {
public:
    string dotPath;
    string dotName;

    int nEdges = 0;
    int nNodes = 0;

    //Adjacency for successors
    vector<vector<bool> > successors;
    //Adjacency for predecessors
    vector<vector<bool> > predecessors;
    //Edges list
    vector<pair<int, int> > gEdges;
    //nodes List
    vector<int> gNodes;

    //adjacency list
    unordered_map<int, vector<int> > adjList;

    vector<int> nSuccV;
    vector<int> nPredV;

    vector<int> inputNodes;
    vector<int> outputNodes;
    vector<int> otherNodes;

    vector<int> longestPath;

    Graph(const string &dotPath, const string &dotName);

    //void readGraphDataStr();

    void readEdges();

    void readNodes();

    void readSuccPred();

    void readGraphData();

    vector<pair<int, int> > getEdgesDepthFirst();

    vector<pair<int, int> > getEdgesDepthFirstPriority();

    vector<pair<int, int> > getEdgesZigzag(vector<pair<int, int> > &convergence);

    vector<pair<int, int> > clearEdges(const vector<pair<int, int> > &edges);

    void dfs(int idx, const vector<vector<int> > &adj, vector<bool> &visited, vector<int> &topo_order);

    void findLongestPath();
};
#endif
