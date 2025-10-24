#ifndef GRAPH_H
#define GRAPH_H

#include <common/definitions.h>


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
    std::vector<std::vector<long> > succList;
    std::vector<std::vector<long> > predList;

    std::vector<long> nSuccV;
    std::vector<long> nPredV;

    std::vector<long> inputNodes;
    std::vector<long> outputNodes;
    std::vector<long> disconnectedNodes;
    std::vector<long> innerNodes;

    std::vector<long> asap;
    std::vector<long> alap;
    std::vector<long> slack;


    Graph(const std::string &dotPath, const std::string &dotName, bool str = false);

    void readGraphDataStr();

    void isolateMultiInputOutputs();

    void updateG();

    void readEdgesNodes();

    void readAdjList();

    void readSuccPred();

    void readAsapAlap();

    void readTypeOfNodes();

    std::vector<std::pair<long, long> > getEdgesHybrid();

    std::vector<std::pair<long, long> > getEdgesDepthFirst(bool criticalPriority = true);

    std::vector<std::pair<long, long> > getEdgesZigzag(
        std::vector<std::pair<long, long> > &convergence,
        std::vector<std::tuple<long, long, std::string> > *edgeTypes = nullptr);

    std::vector<std::pair<long, long> > clearZigZagEdges(const std::vector<std::pair<long, long> > &edges) const;

    /*void dfs(long idx, const std::vector<std::vector<long> > &adj, std::vector<bool> &visited,
             std::vector<long> &topo_order);*/

    void saveToDot(const std::string &filename);
};
#endif
