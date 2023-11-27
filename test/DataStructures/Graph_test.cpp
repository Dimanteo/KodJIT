#include <DataStructures/Graph.hpp>

#include <gtest/gtest.h>
#include <set>
#include <vector>

namespace koda {

namespace Tests {

class TestGraph {
  using EdgeList = std::vector<std::set<size_t>>;

  EdgeList m_preds;
  EdgeList m_succs;

public:
  TestGraph(size_t n_nodes) : m_preds(n_nodes), m_succs(n_nodes) {}

  void add_edge(size_t from, size_t to) {
    m_succs[from].insert(to);
    m_preds[to].insert(from);
  }

  using NodeId = size_t;
  using PredIterator = std::set<size_t>::iterator;
  using SuccIterator = std::set<size_t>::iterator;

  static PredIterator predBegin(TestGraph &this_, NodeId node) { return this_.m_preds[node].begin(); }

  static PredIterator predEnd(TestGraph &this_, NodeId node) { return this_.m_preds[node].end(); }

  static SuccIterator succBegin(TestGraph &this_, NodeId node) { return this_.m_succs[node].begin(); }

  static SuccIterator succEnd(TestGraph &this_, NodeId node) { return this_.m_succs[node].end(); }
};


TEST(GraphTests, DFSFork) {
/*
┌───┐     ┌───┐
│ 0 │ ──▶ │ 1 │
└───┘     └───┘
  │
  │
  ▼
┌───┐
│ 2 │
└───┘
*/

  TestGraph graph(3);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);

  std::vector<size_t> reference_path;
  auto reference_inserter = std::back_inserter(reference_path);
  *reference_inserter = 0;
  std::for_each(TestGraph::succBegin(graph, 0), TestGraph::succEnd(graph, 0),
                [&reference_inserter](size_t node) { *reference_inserter = node; });

  std::vector<size_t> path;
  auto path_inserter = std::back_inserter(path);
  auto visitor = [&path_inserter](size_t node) { *path_inserter = node; };

  visit_dfs(graph, 0, visitor);

  ASSERT_EQ(reference_path.size(), path.size());
  ASSERT_EQ(path[0], reference_path[0]);

  for (size_t i = 1; i < path.size(); ++i) {
    bool cond = (path[i] == reference_path[1] || path[i] == reference_path[2]);
    ASSERT_TRUE(cond);
  }
}


TEST(GraphTests, DFSLinear) {
/*
┌───┐     ┌───┐     ┌───┐     ┌───┐
│ 0 │ ──▶ │ 1 │ ──▶ │ 2 │ ──▶ │ 3 │
└───┘     └───┘     └───┘     └───┘
*/

  TestGraph graph(4);
  graph.add_edge(0, 1);
  graph.add_edge(1, 2);
  graph.add_edge(2, 3);

  std::vector<size_t> reference_path{0, 1, 2, 3};

  std::vector<size_t> path;
  auto path_inserter = std::back_inserter(path);
  auto visitor = [&path_inserter](size_t node) { *path_inserter = node; };

  visit_dfs(graph, 0, visitor);

  ASSERT_EQ(reference_path.size(), path.size());

  for (size_t i = 0; i < path.size(); ++i) {
    ASSERT_EQ(path[i], reference_path[i]);
  }
}

TEST(GraphTests, DFSCycle) {
/*
            ┌─────────┐
            ▼         │
┌───┐     ┌───┐     ┌───┐     ┌───┐
│ 0 │ ──▶ │ 1 │ ──▶ │ 2 │ ──▶ │ 3 │
└───┘     └───┘     └───┘     └───┘
  ▲                             │
  └─────────────────────────────┘
*/
  TestGraph graph(4);
  graph.add_edge(0, 1);
  graph.add_edge(1, 2);
  graph.add_edge(2, 1);
  graph.add_edge(2, 3);
  graph.add_edge(3, 0);

  std::vector<size_t> reference_path{0, 1, 2, 3};

  std::vector<size_t> path;
  auto path_inserter = std::back_inserter(path);
  auto visitor = [&path_inserter](size_t node) { *path_inserter = node; };

  visit_dfs(graph, 0, visitor);

  ASSERT_EQ(reference_path.size(), path.size());

  for (size_t i = 0; i < path.size(); ++i) {
    ASSERT_EQ(path[i], reference_path[i]);
  }

}

} // namespace Tests

} // namespace koda