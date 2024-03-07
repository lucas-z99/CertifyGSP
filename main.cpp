#include "dfs.hpp"

int main(int argc, char *argv[])
{
    using namespace std;

    //   input   ------------------------------------

    // ios::sync_with_stdio(false);
    string _file = "7V.txt";
    if (argc > 1)
        _file = argv[1];

    ifstream is;
    is.open(_file);
    cout << "\nReading from: " << _file << "\n";

    //   container   ------------------------------------

    Graph graph;

    is >> graph.vCount >> graph.eCount; // for now line 1 is no. of vertices / edges

    graph.dfsRank.resize(graph.vCount, -1);
    graph.parent.resize(graph.vCount, -1);
    graph.nDescendants.resize(graph.vCount, 1);
    graph.ear.resize(graph.vCount, pair<int, int>(-1, -1));

    auto edges = get_edges(graph, is);
    init_adjacency(graph, edges);

    //   business logic   ------------------------------------

    atomic_thread_fence(memory_order_seq_cst); // measure the start time with a barrier that prevents the compiler from reordering
    auto time_start = chrono::steady_clock::now();
    atomic_thread_fence(memory_order_seq_cst);

    genCS(0, graph);

    atomic_thread_fence(memory_order_seq_cst); // measure the end time with a barrier that prevents the compiler from reordering
    auto time_end = chrono::steady_clock::now();
    atomic_thread_fence(memory_order_seq_cst);

    chrono::duration<double, nano> time_elapsed = chrono::duration_cast<chrono::nanoseconds>(time_end - time_start);
    cout << "\ntime elapsed is: " << setprecision(4) << time_elapsed.count() / 1000 << "μs\n\n";

    //   print   ------------------------------------

    // for (int i = 0; i < graph.vCount; i++) // adjacency
    // {
    //     cout << "List " << i << " is: ";
    //     for (int j = graph.adjAdd[i]; j < graph.adjAdd[i + 1]; j++)
    //     {
    //         char c = (isTree(graph.adj[j], i, graph) ? 'T' : 'B');
    //         cout << c << graph.adj[j] << " ";
    //     }
    //     cout << '\n';
    // }

    // cout << "\n";

    for (size_t i = 0; i < graph.dfsRank.size(); ++i) // results
    {
        cout << "Vertex " << i
             << " | DFS " << graph.dfsRank[i]
             << " | Parent " << graph.parent[i]
             << " | nDescendants " << graph.nDescendants[i]
             << " | ear " << graph.ear[i].first << ',' << graph.ear[i].second << "\n";
    }

    cout << "\n";
}
