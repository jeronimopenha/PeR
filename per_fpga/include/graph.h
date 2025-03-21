//
// Created by jeronimo on 27/12/24.
//

#ifndef GRAPH_H
#define GRAPH_H

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
    //Edges list
    vector<pair<int, int> > gEdges;
    //input nodes
    vector<int> nSuccV;
    //output nodes
    vector<int> nPredV;
    vector<int> inputNodes;
    vector<int> outputNodes;


    Graph(const string &dotPath, const string &dotName);

    //void graphClearData();

    void getGraphDataStr();

    void getGraphDataInt();

    vector<int> getInOutPos();

    vector<pair<int, int> > getEdgesDepthFirst();

    vector<pair<int, int> > getEdgesDepthFirstPriority();

    vector<pair<int, int> > getEdgesZigzag(vector<pair<int, int> > &convergence);

    vector<pair<int, int> > clearEdges(const vector<pair<int, int> > &edges);

    unordered_map<string, vector<pair<int, int> > > get_graph_annotations(
        const vector<pair<int, int> > &edges,
        const vector<pair<int, int> > &convergences
    );

    void dfs(int idx, const vector<vector<int> > &adj, vector<bool> &visited, vector<int> &topo_order);

    vector<int> findLongestPath();
};


#endif //GRAPH_H
