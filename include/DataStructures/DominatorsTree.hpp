#pragma once

#include <DataStructures/Graph.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace koda {

template <typename Graph> class DominatorTree final {
  using NodeId = typename GraphTraits<Graph>::NodeId;
  using DomNodeContainer = std::unordered_set<NodeId>;

  struct DomNode {
    DomNodeContainer m_succ;
    DomNodeContainer m_preds;
  };

  std::unordered_map<NodeId, DomNode> m_tree;

public:
  using iterator = typename DomNodeContainer::iterator;

  void set_domination(NodeId dominator, NodeId dominated) {
    m_tree[dominator].m_succ.insert(dominated);
    m_tree[dominated].m_preds.insert(dominator);
  }

  bool is_dominator_of(NodeId dominator, NodeId dominated) {
    auto &&dominated_nodes = m_tree[dominator].m_succ;
    return dominated_nodes.find(dominated) != dominated_nodes.end();
  }

  bool is_domination_computed(NodeId node) {
    return m_tree.find(node) != m_tree.end();
  }

  iterator dominated_by_begin(NodeId node) { return m_tree[node].m_preds.begin(); }
  iterator dominated_by_end(NodeId node) { return m_tree[node].m_preds.end(); }

  iterator dominator_of_begin(NodeId node) { return m_tree[node].m_succ.begin(); }
  iterator dominator_of_end(NodeId node) { return m_tree[node].m_succ.end(); }
};

template <typename Graph> class DominatorTreeBuilder final {
  using Traits = GraphTraits<Graph>;
  using NodeId = typename Traits::NodeId;

  NodeId m_entry;
  std::vector<NodeId> m_all_nodes;
  DominatorTree<Graph> *m_tree;

  void find_dominated_by(Graph &graph, NodeId dominator) {
    std::unordered_set<NodeId> path;

    auto visitor = [dominator, &path](NodeId node) {
      if (node == dominator) {
        return false;
      }
      path.insert(node);
      return true;
    };

    visit_dfs_conditional(graph, m_entry, visitor);

    path.insert(dominator);
    for (auto &&node : m_all_nodes) {
      if (path.find(node) == path.end())
        m_tree->set_domination(dominator, node);
    }
  }

  void build(Graph &graph, NodeId entry, DominatorTree<Graph> &tree) {
    m_entry = entry;
    m_tree = &tree;
    for (auto &&node : m_all_nodes) {
      find_dominated_by(graph, node);
    }
  }

public:
  void build_tree(Graph &graph, NodeId entry, DominatorTree<Graph> &tree) {
    m_all_nodes.clear();
    auto nodes_inserter = std::back_inserter(m_all_nodes);
    visit_dfs(graph, entry, [&nodes_inserter](NodeId node) { *nodes_inserter = node; });
    build(graph, entry, tree);
  }

  template <typename NodeIt>
  void build_tree(Graph &graph, NodeId entry, DominatorTree<Graph> &tree, NodeIt begin, NodeIt end) {
    m_all_nodes.clear();
    std::copy(begin, end, std::back_inserter(m_all_nodes));
    build(graph, entry, tree);
  }
};

} // namespace koda