// usage: ./dfs2 <input_graph>
#include <atomic>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <chrono>
#include <iomanip>
#include <string>

using std::pair;
using std::vector;

class Graph
{
public:
    int vertex_count = 0;       // |V| size of graph
    int edge_count = 0;         // |E| number of edges
    vector<int> parent{};       // parent[v] = index of parent of v (dfs tree)
    vector<int> nDescendants{}; // no. of descendants (including itself)
    vector<int> dfsRank{};      // ?????   holds the number that represents in the order that the dfs accessed each vertex in the graph in.
    vector<int> adj{};          // ?????   adj[0] contains the vector of Edges that are connected to vertex 0. Edge contains target vertex and tree edge boolean.
    vector<int> adjAddress{};
    vector<pair<int, int>> ear{}; // holds ear decomposition data. pair.first must be source, pair.second must be sink
};

void create_adjacency_list(const Graph &, vector<pair<int, int>> const &);
vector<pair<int, int>> create_edges(const Graph &g, std::ifstream &s);
bool isAncestor(int const &, int const &, Graph const &);
bool lexiCompare(int const &, int const &, int const &, int const &, Graph const &);
bool isTree(int const &, int const &, Graph const &);
void genCS(int const &, Graph &);

std::string test_data_file = "7V.txt";

int main(int argc, char *argv[])
{
    using namespace std;
    // ios::sync_with_stdio(false);

    Graph graph;
    ifstream is;

    if (argc > 1)
        is.open(argv[1]);
    else
        is.open(test_data_file);

    is >> graph.vertex_count >> graph.edge_count;

    // graph.dfsRank = vector<int>(graph.v_count, -1);
    // graph.dfsRank.shrink_to_fit();
    // graph.parent = vector<int>(graph.v_count, -1);
    // graph.parent.shrink_to_fit();
    // graph.nDescendants = vector<int>(graph.v_count, 1);
    // graph.nDescendants.shrink_to_fit();
    // graph.ear = vector<pair<int, int>>(graph.v_count, pair<int, int>(-1, -1));
    // graph.ear.shrink_to_fit();
    // graph.adj.reserve(graph.e_count * 2);
    // graph.adjAddress.reserve(graph.v_count + 1);

    auto edges = create_edges(graph, is);
    // create_adjacency_list(graph.vertex_count, edges, graph.adj, graph.adjAddress);
    create_adjacency_list(graph, edges);

    // measure the start time with a barrier that prevents the compiler from reordering
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto time_start = chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);

    genCS(0, graph);

    // measure the end time with a barrier that prevents the compiler from reordering
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto time_end = chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);

    chrono::duration<double, nano> time_elapsed = chrono::duration_cast<chrono::nanoseconds>(time_end - time_start);
    cout << "time elapsed is: " << setprecision(4) << time_elapsed.count() / 1000 << "μs\n";

    // Print adjacency list for debugging
    for (int i = 0; i < graph.vertex_count; i++)
    {
        cout << "List " << i << " is: ";
        for (int j = graph.adjAddress[i]; j < graph.adjAddress[i + 1]; j++)
        {
            char c = (isTree(graph.adj[j], i, graph) ? 'T' : 'B');
            cout << c << graph.adj[j] << " ";
        }
        cout << '\n';
    }
    cout << '\n';

    // Print DFS ranks for testing
    cout << '\n';
    for (int i = 0; i < graph.dfsRank.size(); ++i)
    {
        cout << "Vertex " << i << " | DFS " << graph.dfsRank[i] << " | Parent " << graph.parent[i] << " | nDescendants " << graph.nDescendants[i] << " | ear " << graph.ear[i].first << ',' << graph.ear[i].second << "\n";
    }

    return 0;
}

void genCS(int const &starting_vertex, Graph &g)
{
    // stack<int, std::vector<int>> the_stack;
    std::vector<int> the_stack;
    the_stack.reserve(g.vertex_count);
    the_stack.push_back(starting_vertex);
    int dfsNumber = 1;
    g.dfsRank[starting_vertex] = dfsNumber++;

    while (!the_stack.empty())
    {
        int topOfStack = the_stack.back();
        bool descend = false;
        for (int i = g.adjAddress[topOfStack]; i < g.adjAddress[topOfStack + 1]; i++)
        {
            int w = g.adj[i];
            if (g.dfsRank[w] == -1)
            {
                g.parent[w] = topOfStack;
                g.dfsRank[w] = dfsNumber++;
                the_stack.push_back(w);
                descend = true;
                break;
            }
            else if (g.dfsRank[w] < g.dfsRank[topOfStack] && w != g.parent[topOfStack])
            {
                // back edge detected
                g.ear[topOfStack].first = topOfStack;
                g.ear[topOfStack].second = w;
            }
        }
        if (descend)
        {
            continue;
        }

        if (topOfStack != starting_vertex)
        {
            g.nDescendants[g.parent[topOfStack]] += g.nDescendants[topOfStack];
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
        if (g.dfsRank[topOfStack] > 2 && lexiCompare(g.ear[topOfStack].first, g.ear[topOfStack].second, g.ear[g.parent[topOfStack]].first, g.ear[g.parent[topOfStack]].second, g))
        {
            g.ear[g.parent[topOfStack]].first = g.ear[topOfStack].first;
            g.ear[g.parent[topOfStack]].second = g.ear[topOfStack].second;
        }
        the_stack.pop_back();
    }
}

void create_adjacency_list(Graph &g, vector<pair<int, int>> const &edges)
{
    // for (int i = 0; i < edges.size(); i++)
    // {
    //     g.adjAddress[edges[i].first + 1]++;
    //     g.adjAddress[edges[i].second + 1]++;
    // }
    for (auto e : edges)
    {
        g.adjAddress[e.first + 1]++;
        g.adjAddress[e.second + 1]++;
    }

    vector<int> temp(g.vertex_count + 1, 0);
    for (int i = 1; i <= g.vertex_count; i++)
    {
        g.adjAddress[i] += g.adjAddress[i - 1];
        temp[i] = g.adjAddress[i];
    }

    // for (int i = 0; i < edges.size(); i++)
    // {
    //     int x = edges[i].first;
    //     int y = edges[i].second;
    //     g.adj[temp[x]++] = y;
    //     g.adj[temp[y]++] = x;
    // }
    for (auto e : edges)
    {
        g.adj[g.adjAddress[e.first]++] = e.second;
        g.adj[g.adjAddress[e.second]++] = e.first;
    }
}

vector<pair<int, int>> create_edges(const Graph &g, std::ifstream &s)
{
    vector<pair<int, int>> edges;
    edges.reserve(g.edge_count);

    for (int i = 0; i < g.edge_count; ++i)
    {
        int x, y;
        s >> x >> y;
        edges.emplace_back((x > y ? y : x), (x > y ? x : y));
    }

    return edges;
}

bool isAncestor(int const &a, int const &b, Graph const &gD)
{
    return (gD.dfsRank[a] <= gD.dfsRank[b] && gD.dfsRank[b] < (gD.dfsRank[a] + gD.nDescendants[a]));
}

// return true if back-edge (q <-- p) is smaller than (y <-- x)
// return false if (y <-- x) is smaller than (q <-- p)
bool lexiCompare(int const &p, int const &q, int const &x, int const &y, Graph const &gD)
{
    return x == -1 || (gD.dfsRank[q] < gD.dfsRank[y]) || ((gD.dfsRank[q] == gD.dfsRank[y]) && (gD.dfsRank[p] < gD.dfsRank[x]) && !isAncestor(p, x, gD)) || ((gD.dfsRank[q] == gD.dfsRank[y]) && isAncestor(x, p, gD));
}

bool isTree(int const &a, int const &b, Graph const &gD)
{
    return (gD.parent[a] == b || gD.parent[b] == a);
}