#ifndef GRAPH_H
#define GRAPH_H

#include <common/util.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>


class Graph {
public:
    std::string dotPath;
    std::string dotName;

    long nEdges = 0;
    long nNodes = 0;

    //Adjacency for successors
    std::vector<std::vector<bool> > successors;
    //Adjacency for predecessors
    std::vector<std::vector<bool> > predecessors;
    //Edges list
    std::vector<std::pair<long, long> > gEdges;
    //Edges list
    std::vector<std::pair<std::string, std::string> > gEdgesStr;
    //nodes List
    std::vector<long> gNodes;

    //adjacency list
    std::unordered_map<long, std::vector<long> > adjList;

    std::vector<long> nSuccV;
    std::vector<long> nPredV;

    std::vector<long> inputNodes;
    std::vector<long> outputNodes;
    std::vector<long> otherNodes;


    Graph(const std::string &dotPath, const std::string &dotName, bool str = false);

    void readGraphDataStr();

    void isolateMultiInputOutputs();

    void updateG();

    void readEdgesNodes();

    void readAdjList();

    void readIONodes();

    void readSuccPred();

    std::vector<std::pair<long, long> > getEdgesDepthFirst();

    std::vector<std::pair<long, long> > getEdgesZigzag(
        std::vector<std::pair<long, long> > &convergence,
        std::vector<std::tuple<long, long, std::string> > *edgeTypes = nullptr);

    std::vector<std::pair<long, long> > clearEdges(const std::vector<std::pair<long, long> > &edges) const;

    void dfs(long idx, const std::vector<std::vector<long> > &adj, std::vector<bool> &visited,
             std::vector<long> &topo_order);
};
#endif
