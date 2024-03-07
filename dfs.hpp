#pragma once

#include <atomic>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <chrono>
#include <iomanip>
#include <string>
#include <algorithm>

using std::pair;
using std::vector;

class Graph
{
public:
    int vCount = 0; // vertex count
    int eCount = 0; // edge count

    vector<int> parent{};         // DFS parent
    vector<int> nDescendants{};   // no. of descendants (including self)
    vector<int> dfsRank{};        // the order DFS visits this vertex
    vector<int> adj{};            // ?? adj[0] contains the vector of Edges that are connected to vertex 0. Edge contains target vertex and tree edge boolean.
    vector<int> adjAdd{};         // ??
    vector<pair<int, int>> ear{}; // ear decomposition. pair.first must be source, pair.second must be sink
};

bool isAncestor(int const &a, int const &b, Graph const &g)
{
    return (g.dfsRank[a] <= g.dfsRank[b] && g.dfsRank[b] < (g.dfsRank[a] + g.nDescendants[a]));
}

// return true if back-edge (q <-- p) is smaller than (y <-- x)
// return false if (y <-- x) is smaller than (q <-- p)
bool lexiCompare(int const &p, int const &q, int const &x, int const &y, Graph const &g)
{
    return x == -1 || (g.dfsRank[q] < g.dfsRank[y]) || ((g.dfsRank[q] == g.dfsRank[y]) && (g.dfsRank[p] < g.dfsRank[x]) && !isAncestor(p, x, g)) || ((g.dfsRank[q] == g.dfsRank[y]) && isAncestor(x, p, g));
}

bool isTree(int const &a, int const &b, Graph const &g)
{
    return (g.parent[a] == b || g.parent[b] == a);
}

vector<pair<int, int>> get_edges(Graph &graph, std::ifstream &is)
{
    using namespace std;
    vector<pair<int, int>> edges;
    for (int i = 0; i < graph.eCount; ++i)
    {
        int x, y;
        is >> x >> y;
        edges.emplace_back(min(x, y), max(x, y));
    }

    return edges;
}

void init_adjacency(Graph &graph, vector<pair<int, int>> edges)
{
    // count edges?
    graph.adjAdd.resize(graph.vCount + 1); // +1 for root vertex?
    graph.adj.resize(graph.eCount * 2);

    for (auto e : edges)
    {
        graph.adjAdd[e.first + 1]++; // again +1
        graph.adjAdd[e.second + 1]++;
    }

    // errr what?
    vector<int> temp(graph.vCount + 1, 0);
    for (int i = 1; i <= graph.vCount; i++)
    {
        graph.adjAdd[i] += graph.adjAdd[i - 1];
        temp[i] = graph.adjAdd[i];
    }

    for (auto e : edges)
    {
        graph.adj[temp[e.first]++] = e.second;
        graph.adj[temp[e.second]++] = e.first;
    }
}

void genCS(int const &starting_vertex, Graph &graph)
{
    std::vector<int> _stack(graph.vCount);
    _stack.push_back(starting_vertex);
    int dfsNumber = 1;
    graph.dfsRank[starting_vertex] = dfsNumber++;

    while (!_stack.empty())
    {
        int topOfStack = _stack.back();
        bool descend = false;
        for (int i = graph.adjAdd[topOfStack]; i < graph.adjAdd[topOfStack + 1]; i++)
        {
            int w = graph.adj[i];
            if (graph.dfsRank[w] == -1)
            {
                graph.parent[w] = topOfStack;
                graph.dfsRank[w] = dfsNumber++;
                _stack.push_back(w);
                descend = true;
                break;
            }
            else if (graph.dfsRank[w] < graph.dfsRank[topOfStack] && w != graph.parent[topOfStack])
            {
                // back edge detected
                graph.ear[topOfStack].first = topOfStack;
                graph.ear[topOfStack].second = w;
            }
        }
        if (descend)
        {
            continue;
        }

        if (topOfStack != starting_vertex)
        {
            graph.nDescendants[graph.parent[topOfStack]] += graph.nDescendants[topOfStack];
        }
        /*
        dfsRank[topOfStack] > 2; this is an easier way of writing
        topOfStack != starting_vertex && parent[topOfStack] != starting_vertex

        both of those are necessary because:
        1. if I only check topOfStack != starting_vertex, then when the vertex with the root r as parent is backed into as the search
            is reversing up the tree, the if condition will succeed and gD.ear[gD.parent[topOfStack]] will run which produces an error
            because the result is -1 and the function will use that parameter to find gD.dfsRank[-1] which doesn't exist.
        2. if I only check parent[topOfStack] != starting_vertex, then when the root r is backed into as the search reverses up the tree
            the if condition will succeed and gD.ear[gD.parent[topOfStack]] will run which produces an error because gD.ear[-1] doesn't
            exist.
        */
        if (graph.dfsRank[topOfStack] > 2 && lexiCompare(graph.ear[topOfStack].first, graph.ear[topOfStack].second, graph.ear[graph.parent[topOfStack]].first, graph.ear[graph.parent[topOfStack]].second, graph))
        {
            graph.ear[graph.parent[topOfStack]].first = graph.ear[topOfStack].first;
            graph.ear[graph.parent[topOfStack]].second = graph.ear[topOfStack].second;
        }
        _stack.pop_back();
    }
}
