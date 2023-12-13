#include <DataStructures/DominatorTree.hpp>
#include <DataStructures/Graph.hpp>

#include <fstream>
#include <gtest/gtest.h>
#include <set>
#include <vector>

namespace koda {

namespace Tests {

class TestGraph {
  using EdgeList = std::vector<std::set<size_t>>;

  EdgeList m_preds;
  EdgeList m_succs;
  size_t m_size;

public:
  TestGraph(size_t n_nodes) : m_preds(n_nodes), m_succs(n_nodes), m_size(n_nodes) {}

  void add_edge(size_t from, size_t to) {
    m_succs[from].insert(to);
    m_preds[to].insert(from);
  }

  size_t size() const { return m_size; }

  // Graph traits
  using NodeId = size_t;
  using PredIterator = std::set<size_t>::iterator;
  using SuccIterator = std::set<size_t>::iterator;

  static PredIterator pred_begin(TestGraph &this_, NodeId node) { return this_.m_preds[node].begin(); }

  static PredIterator pred_end(TestGraph &this_, NodeId node) { return this_.m_preds[node].end(); }

  static SuccIterator succ_begin(TestGraph &this_, NodeId node) { return this_.m_succs[node].begin(); }

  static SuccIterator succ_end(TestGraph &this_, NodeId node) { return this_.m_succs[node].end(); }

  // Printable graph traits
  static std::string node_to_string(TestGraph &this_, NodeId node) {
    (void)this_;
    return std::to_string(node);
  }
};

void dump_graph_and_dom_tree(TestGraph &graph, DominatorTree<size_t> &tree, std::string filename) {
  std::ofstream dot_log(filename + ".dot", std::ios_base::out);
  GraphPrinter::print_dot(graph, 1, dot_log);
  dot_log.close();

  dot_log.open(filename + "_DomTree.dot", std::ios_base::out);
  GraphPrinter::print_dot(tree, 1, dot_log);
  dot_log.close();
}

void compare_graphs(TestGraph &first, TestGraph &second, size_t entry) {
  std::vector<size_t> first_path, second_path;

  ASSERT_EQ(first.size(), second.size());

  auto path_inserter = std::back_inserter(first_path);
  auto visitor_f = [&path_inserter](size_t node) { *path_inserter = node; };
  visit_dfs(first, entry, visitor_f);

  path_inserter = std::back_inserter(second_path);
  auto visitor_s = [&path_inserter](size_t node) { *path_inserter = node; };
  visit_dfs(second, entry, visitor_s);

  ASSERT_EQ(first_path.size(), second_path.size());
  for (size_t i = entry; i < first.size(); ++i) {
    ASSERT_EQ(first_path[i], second_path[i]);
  }
}

void verify_tree(TestGraph &ref, DominatorTree<size_t> &tree, size_t entry) {
  std::vector<size_t> ref_path;

  auto inserter = std::back_inserter(ref_path);
  auto visitor = [&inserter](size_t node) { *inserter = node; };
  visit_dfs(ref, entry, visitor);

  ASSERT_EQ(ref_path.size(), tree.size());
  std::vector<size_t> ref_buf, tree_buf;
  for (auto &&node : ref_path) {
    ASSERT_TRUE(tree.contains(node));
    ref_buf.clear();
    tree_buf.clear();
    std::copy(ref.pred_begin(ref, node), ref.pred_end(ref, node), std::back_inserter(ref_buf));
    std::copy(tree.pred_begin(node), tree.pred_end(node), std::back_inserter(tree_buf));
    ASSERT_EQ(tree_buf.size(), ref_buf.size());
    ASSERT_TRUE(std::equal(ref_buf.begin(), ref_buf.end(), tree_buf.begin(), tree_buf.end()));
    ref_buf.clear();
    tree_buf.clear();
    std::copy(ref.succ_begin(ref, node), ref.succ_end(ref, node), std::back_inserter(ref_buf));
    std::copy(tree.children_begin(node), tree.children_end(node), std::back_inserter(tree_buf));
    std::sort(ref_buf.begin(), ref_buf.end());
    std::sort(tree_buf.begin(), tree_buf.end());
    ASSERT_EQ(tree_buf.size(), ref_buf.size());
    ASSERT_TRUE(std::equal(ref_buf.begin(), ref_buf.end(), tree_buf.begin(), tree_buf.end()));
  }
}

TEST(GraphTests, DFSLoop) {
  TestGraph graph(2);
  graph.add_edge(0, 1);
  graph.add_edge(1, 1);

  std::vector<size_t> path;
  auto path_inserter = std::back_inserter(path);
  auto visitor = [&path_inserter](size_t node) { *path_inserter = node; };

  visit_dfs(graph, 0, visitor);

  ASSERT_EQ(graph.size(), path.size());
  ASSERT_EQ(path.front(), 0);
  ASSERT_EQ(path.back(), 1);
}

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
  std::for_each(TestGraph::succ_begin(graph, 0), TestGraph::succ_end(graph, 0),
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

TEST(GraphTests, DFSCycleBackwards) {
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

  std::vector<size_t> reference_path{3, 2, 1, 0};

  std::vector<size_t> path;
  auto path_inserter = std::back_inserter(path);
  auto visitor = [&path_inserter](size_t node) { *path_inserter = node; };

  visit_dfs<true>(graph, 3, visitor);

  ASSERT_EQ(reference_path.size(), path.size());

  for (size_t i = 0; i < path.size(); ++i) {
    ASSERT_EQ(path[i], reference_path[i]);
  }
}

TEST(GraphTests, RPOLongPath) {
  /*
    ┌───────────────────┐
    │                   ▼
  ┌───┐     ┌───┐     ┌───┐     ┌───┐
  │ 0 │ ──▶ │ 2 │ ──▶ │ 1 │ ──▶ │ 3 │
  └───┘     └───┘     └───┘     └───┘
  */
  constexpr size_t graph_size = 4;

  TestGraph graph(graph_size);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 3);
  graph.add_edge(2, 1);

  std::vector<size_t> path;
  auto path_inserter = std::back_inserter(path);
  auto visitor = [&path_inserter](size_t node) { *path_inserter = node; };

  visit_rpo(graph, 0, visitor);

  std::vector<size_t> ref_path = {0, 2, 1, 3};
  ASSERT_EQ(path.size(), ref_path.size());
  for (size_t i = 0; i < path.size(); i++) {
    ASSERT_EQ(path[i], ref_path[i]);
  }
}

TEST(GraphTests, RPOForkJoin) {
  /*
  ┌───┐     ┌───┐     ┌───┐
  │ 0 │ ──▶ │ 1 │ ──▶ │ 3 │
  └───┘     └───┘     └───┘
    │                   ▲
    │                   │
    ▼                   │
  ┌───┐                 │
  │ 2 │ ────────────────┘
  └───┘
  */
  constexpr size_t graph_size = 4;

  TestGraph graph(graph_size);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 3);
  graph.add_edge(2, 3);

  std::vector<size_t> rpo;
  auto inserter = std::back_inserter(rpo);
  auto visitor = [&inserter](size_t node) { *inserter = node; };

  visit_rpo(graph, 0, visitor);

  ASSERT_EQ(rpo.size(), graph_size);
  ASSERT_EQ(rpo.front(), 0);
  ASSERT_EQ(rpo.back(), 3);

  std::ofstream dot_log("RPOForkJoin.dot", std::ios_base::out);
  GraphPrinter::print_dot(graph, 0, dot_log);
  dot_log.close();
}

TEST(DomTreeTests, domTreeSimple) {
  constexpr size_t graph_size = 5;

  TestGraph graph(graph_size);
  graph.add_edge(4, 0);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 3);
  graph.add_edge(2, 3);

  DominatorTree<size_t> tree(graph_size);
  DominatorTreeBuilder<TestGraph> builder;
  builder.build_tree(graph, 4, tree);

  for (size_t i = 0; i < graph_size; i++) {
    ASSERT_TRUE(tree.contains(i));
  }

  ASSERT_TRUE(tree.is_dominator_of(4, 0));
  ASSERT_TRUE(tree.is_dominator_of(0, 1));
  ASSERT_TRUE(tree.is_dominator_of(0, 2));
  ASSERT_TRUE(tree.is_dominator_of(0, 3));

  ASSERT_FALSE(tree.is_dominator_of(1, 3));
  ASSERT_FALSE(tree.is_dominator_of(2, 3));

  ASSERT_FALSE(tree.is_dominator_of(2, 1));

  std::set<size_t> doms;
  for (auto &&dom = tree.get(3).begin(); dom != tree.get(3).end(); ++dom) {
    doms.insert(*dom);
  }
  ASSERT_EQ(doms.size(), 2);
  ASSERT_EQ(*doms.begin(), 0);
  ASSERT_EQ(*++doms.begin(), 4);
}

TEST(DomTreeTests, domTreeExample1) {
  /*
            ┌───┐
            │ 1 │
            └───┘
              │
              │
              ▼
  ┌───┐     ┌───┐
  │ 3 │ ◀── │ 2 │
  └───┘     └───┘
    │         │
    │         │
    │         ▼
    │       ┌───┐     ┌───┐
    │       │ 6 │ ──▶ │ 7 │
    │       └───┘     └───┘
    │         │         │
    │         │         │
    │         ▼         │
    │       ┌───┐       │
    │       │ 5 │       │
    │       └───┘       │
    │         │         │
    │         │         │
    │         ▼         │
    │       ┌───┐       │
    └─────▶ │ 4 │ ◀─────┘
            └───┘
  */
  constexpr size_t graph_size = 8;

  TestGraph graph(graph_size);
  graph.add_edge(1, 2);

  graph.add_edge(2, 3);
  graph.add_edge(2, 6);

  graph.add_edge(3, 4);

  graph.add_edge(5, 4);

  graph.add_edge(6, 5);
  graph.add_edge(6, 7);

  graph.add_edge(7, 4);

  DominatorTree<size_t> tree(0);
  DominatorTreeBuilder<TestGraph> dom_builder;
  dom_builder.build_tree(graph, 1, tree);

  TestGraph ref_idom(graph_size);
  ref_idom.add_edge(1, 2);
  ref_idom.add_edge(2, 3);
  ref_idom.add_edge(2, 4);
  ref_idom.add_edge(2, 6);
  ref_idom.add_edge(6, 5);
  ref_idom.add_edge(6, 7);

  verify_tree(ref_idom, tree, 1);
  dump_graph_and_dom_tree(graph, tree, "Example1");
}

TEST(DomTreeTests, domTreeExample2) {
  /*
          ┌────┐
          │ 1  │
          └────┘
            │
            │
            ▼
          ┌────┐
  ┌─────▶ │ 2  │ ─┐
  │       └────┘  │
  │         │     │
  │         │     │
  │         ▼     │
  │       ┌────┐  │
  │       │ 4  │  │
  │       └────┘  │
  │         │     │
  │         │     │
  │         ▼     │
  │       ┌────┐  │
  │    ┌▶ │ 3  │ ◀┘
  │    │  └────┘
  │    │    │
  │    │    │
  │    │    ▼
  │    │  ┌────┐
  │    └─ │ 5  │
  │       └────┘
  │         │
  │         │
  │         ▼
  │       ┌────┐
  │       │ 6  │ ◀┐
  │       └────┘  │
  │         │     │
  │         │     │
  │         ▼     │
  │       ┌────┐  │
  │       │ 7  │ ─┘
  │       └────┘
  │         │
  │         │
  │         ▼
┌───┐     ┌────┐
│ 9 │ ◀── │ 8  │
└───┘     └────┘
            │
            │
            ▼
          ┌────┐
          │ 10 │
          └────┘
            │
            │
            ▼
          ┌────┐
          │ 11 │
          └────┘
  */
  constexpr size_t graph_size = 12;

  TestGraph graph(graph_size);

  graph.add_edge(1, 2);

  graph.add_edge(2, 3);
  graph.add_edge(2, 4);

  graph.add_edge(3, 5);

  graph.add_edge(4, 3);

  graph.add_edge(5, 3);
  graph.add_edge(5, 6);

  graph.add_edge(6, 7);

  graph.add_edge(7, 6);

  graph.add_edge(7, 8);

  graph.add_edge(8, 9);
  graph.add_edge(8, 10);

  graph.add_edge(9, 2);

  graph.add_edge(10, 11);

  DominatorTree<size_t> tree(0);
  DominatorTreeBuilder<TestGraph> dom_builder;
  dom_builder.build_tree(graph, 1, tree);

  TestGraph ref_idom(graph_size);
  ref_idom.add_edge(1, 2);
  ref_idom.add_edge(2, 4);
  ref_idom.add_edge(2, 3);
  ref_idom.add_edge(3, 5);
  ref_idom.add_edge(5, 6);
  ref_idom.add_edge(6, 7);
  ref_idom.add_edge(7, 8);
  ref_idom.add_edge(8, 9);
  ref_idom.add_edge(8, 10);
  ref_idom.add_edge(10, 11);

  verify_tree(ref_idom, tree, 1);
  dump_graph_and_dom_tree(graph, tree, "Example2");
}

TEST(DomTreeTests, domTreeExample3) {
  /*

       ┌────────────────────────┐
       │                        │
       │                 ┌───┐  │
       │                 │ 1 │  │
       │                 └───┘  │
       │                   │    │
       │                   │    │
       │                   ▼    │
     ┌───┐     ┌───┐     ┌───┐  │
     │ 6 │ ◀── │ 5 │ ◀── │ 2 │ ◀┘
     └───┘     └───┘     └───┘
       │         │         │
       │         │         │
       ▼         │         ▼
     ┌───┐       │       ┌───┐
  ┌─ │ 8 │       │       │ 3 │ ◀┐
  │  └───┘       │       └───┘  │
  │    │         │         │    │
  │    │         │         │    │
  │    │         │         ▼    │
  │    │         │       ┌───┐  │
  │    │         └─────▶ │ 4 │  │
  │    │                 └───┘  │
  │    │                   │    │
  │    │                   │    │
  │    │                   ▼    │
  │    │                 ┌───┐  │
  │    └───────────────▶ │ 7 │ ─┘
  │                      └───┘
  │                        │
  │                        │
  │                        ▼
  │                      ┌───┐
  │                      │ 9 │
  │                      └───┘
  │                        ▲
  └────────────────────────┘
  */
  TestGraph graph(10);
  graph.add_edge(1, 2);
  graph.add_edge(2, 5);
  graph.add_edge(2, 3);
  graph.add_edge(3, 4);
  graph.add_edge(4, 7);
  graph.add_edge(5, 4);
  graph.add_edge(5, 6);
  graph.add_edge(6, 8);
  graph.add_edge(6, 2);
  graph.add_edge(7, 9);
  graph.add_edge(7, 3);
  graph.add_edge(8, 7);
  graph.add_edge(8, 9);

  DominatorTree<size_t> tree(0);
  DominatorTreeBuilder<TestGraph> dom_builder;
  dom_builder.build_tree(graph, 1, tree);

  TestGraph ref_idom(graph.size());
  ref_idom.add_edge(1, 2);
  ref_idom.add_edge(2, 3);
  ref_idom.add_edge(2, 4);
  ref_idom.add_edge(2, 5);
  ref_idom.add_edge(2, 7);
  ref_idom.add_edge(2, 9);
  ref_idom.add_edge(5, 6);
  ref_idom.add_edge(6, 8);

  verify_tree(ref_idom, tree, 1);
  dump_graph_and_dom_tree(graph, tree, "Example3");
}

} // namespace Tests

} // namespace koda
