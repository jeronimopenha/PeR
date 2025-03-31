
#ifndef GRAPH_H
#define GRAPH_H

//*******************************
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <list>
#include <unordered_set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

using namespace std;

class Graph {
public:
    string dotPath;
    string dotName;

    int nEdges = 0;
    int nNodes = 0;
    int nCells = 0;
    int nCellsSqrt = 0;

    //Adjacency for successors
    vector<vector<bool> > successors;
    //Adjacency for predecessors
    vector<vector<bool> > predecessors;
    //Neighbors vector
    vector<vector<int> > neighbors;
    //Edges list
    vector<pair<int, int> > gEdges;
    //nodes List
    vector<int> gNodes;
    //input nodes
    vector<int> nSuccV;
    //output nodes
    vector<int> nPredV;
    vector<int> clbNodes;
    vector<int> inputNodes;
    vector<int> outputNodes;

    vector<int> longestPath;


    Graph(const string &dotPath, const string &dotName);

    //void graphClearData();

    void getGraphDataStr();

    void getGraphDataInt();

    vector<int> getInOutPos();

    vector<int> getClbPos();

    vector<pair<int, int> > getEdgesDepthFirst();

    vector<pair<int, int> > getEdgesDepthFirstPriority();

    vector<pair<int, int> > getEdgesZigzag(vector<pair<int, int> > &convergence);

    vector<pair<int, int> > clearEdges(const vector<pair<int, int> > &edges);

    void getIOAnnotations(unordered_map<string, vector<pair<int, int>>>& annotations, const vector<pair<int, int>>& edges);

    unordered_map<string, vector<pair<int, int> > > getGraphAnnotations(
        const vector<pair<int, int> > &edges,
        const vector<pair<int, int> > &convergences
    );

    void dfs(int idx, const vector<vector<int> > &adj, vector<bool> &visited, vector<int> &topo_order);

    void findLongestPath();
};


#endif
