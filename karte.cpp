#include "limits.h"
#include "stdlib.h"
#include <bits/stdc++.h>
#include <climits>
#include <iostream>
#include <list>
#include <queue>
#include <vector>

struct Edge {
  int source, dest;
  int weight = 1;
};

struct EdgeDependency {
  int source, dest, weichenId, weichenStatus;
};

struct Node {
  int vertex, weight;
};

class Graph {
public:
  std::vector<std::vector<Edge>> adjList;
  std::vector<EdgeDependency> deps;

  Graph(std::vector<Edge> const &edges,
        std::vector<EdgeDependency> edgeDependencies, int n) {
    deps = edgeDependencies;
    adjList.resize(n);

    for (Edge const &edge : edges) {
      adjList[edge.source].push_back(edge);
    }
  }
};

std::pair<std::vector<int>, std::vector<std::vector<EdgeDependency>>>
buildPath(std::vector<int> const &prev, Graph graph, int i, int source) {
  std::vector<int> r = {};
  std::vector<std::vector<EdgeDependency>> g = {};
  int o = i;
  int j = 0;
  while (o != source) {
    r.push_back(o);
    std::vector<EdgeDependency> q;
    std::copy_if(graph.deps.begin(), graph.deps.end(), std::back_inserter(q),
                 [&](const EdgeDependency &i) {
                   return i.source == r[j - 1] && i.dest == r[j];
                 });
    std::vector<EdgeDependency> edg;
    edg.insert(edg.end(), q.begin(), q.end());
    g.push_back(edg);
    o = prev[o];
    ++j;
  };
  std::reverse(r.begin(), r.end());
  std::reverse(g.begin(), g.end());
  return std::make_pair(r, g);
};

struct comp {
  bool operator()(const Node &lhs, const Node &rhs) const {
    return lhs.weight > rhs.weight;
  }
};

void findShortestPaths(Graph const &graph, int source, int n) {
  std::priority_queue<Node, std::vector<Node>, comp> min_heap;
  min_heap.push({source, 0});

  std::vector<int> dist(n, INT_MAX);

  dist[source] = 0;

  std::vector<bool> done(n, false);
  done[source] = true;

  std::vector<int> prev(n, -1);

  while (!min_heap.empty()) {
    Node node = min_heap.top();
    min_heap.pop();

    int u = node.vertex;

    for (auto i : graph.adjList[u]) {
      int v = i.dest;
      std::vector<EdgeDependency> q;
      std::copy_if(graph.deps.begin(), graph.deps.end(), std::back_inserter(q),
                   [&](const EdgeDependency &z) {
                     return z.source == i.source && z.dest == i.dest;
                   });
      int weight = 1 + 8 * q.size();

      if (!done[v] && (dist[u] + weight) < dist[v]) {
        dist[v] = dist[u] + weight;
        prev[v] = u;
        min_heap.push({v, dist[v]});
      }
    }

    done[u] = true;
  }

  for (int i = 0; i < n; i++) {
    if (i != source && dist[i] != INT_MAX) {
      auto path = buildPath(prev, graph, i, source);
      /*if (path.first.size() > 6) {std::cout << source << " —> " << i << " & "
      << dist[i] << " & "; for (int j = 0; j < path.first.size(); j++) {
          std::cout << std::to_string(path.first[j]) << " ";
          for (int k = 0; k < path.second[j].size(); k++) {
            std::cout << "(" << path.second[j][k].weichenId << "="
                      << path.second[j][k].weichenStatus << ")"
                      << " ";
          };
        }
        std::cout << "\\\\" << std::endl;
      }*/
      int c = 0;
      for (int j = 0; j < path.first.size(); j++) {
        for (int k = 0; k < path.second[j].size(); k++) {
          c++;
        }
      }
      std::cout << source << " " << i << " " << c;
    };
  }
};

// Mapping
Graph karte() {
  std::vector<Edge> edges = {// äußerer Ring
                             {0, 1},
                             {1, 2},
                             {2, 3},
                             {3, 4},
                             {4, 5},
                             {5, 6},
                             {6, 0},
                             {1, 0},
                             {2, 1},
                             {3, 2},
                             {4, 3},
                             {5, 4},
                             {6, 5},
                             {0, 6},
                             // innerer Ring
                             {7, 8},
                             {8, 9},
                             {9, 10},
                             {10, 11},
                             {11, 12},
                             {12, 13},
                             {13, 7},
                             {8, 7},
                             {9, 8},
                             {10, 9},
                             {11, 10},
                             {12, 11},
                             {13, 12},
                             // Tunnel
                             // Verbindung äußerer/innerer Ring
                             {4, 9},
                             {9, 4},
                             // Verbindung innerer Ring/Tunnel
                             {14, 12},
                             {12, 14}};

  std::vector<EdgeDependency> edgeDeps = {
      {4, 9, 4, 0},   {4, 9, 3, 1},   {9, 4, 4, 0},   {9, 4, 3, 1},
      {3, 4, 4, 1},   {4, 3, 4, 1},   {9, 10, 3, 0},  {10, 9, 3, 0},
      {12, 14, 5, 0}, {14, 12, 5, 0}, {12, 13, 5, 1}, {13, 12, 5, 1}};

  int l = 15;

  Graph r(edges, edgeDeps, l);

  for (int source = 0; source < l; source++) {
    findShortestPaths(r, source, l);
  }

  return r;
};

int main() { karte(); }