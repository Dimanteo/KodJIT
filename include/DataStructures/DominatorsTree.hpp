#pragma once

#include <DataStructures/Graph.hpp>

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace koda {

template <typename NodeId> class DominatorTree final {
  using DomNodeContainer = std::unordered_set<NodeId>;

  struct DomNode {
    NodeId idom;
    DomNodeContainer m_succ;
    DomNodeContainer m_preds;
  };

  std::unordered_map<NodeId, DomNode> m_tree;

public:
  using iterator = typename DomNodeContainer::iterator;

  // Return false if given relation is invalid. In particular if @dominated already dominating @dominator
  bool set_domination(NodeId dominator, NodeId dominated) {
    if (is_domination_computed(dominated) && is_dominator_of(dominated, dominator)) {
      return false;
    }
    m_tree[dominator].m_succ.insert(dominated);
    m_tree[dominated].m_preds.insert(dominator);
    return true;
  }

  bool is_dominator_of(NodeId dominator, NodeId dominated) {
    auto &&dominated_nodes = m_tree[dominator].m_succ;
    return dominated_nodes.find(dominated) != dominated_nodes.end();
  }

  bool is_domination_computed(NodeId node) const { return m_tree.find(node) != m_tree.end(); }

  void set_immediate_dom(NodeId node, NodeId dominator) { m_tree[node].idom = dominator; }

  NodeId get_immediate_dom(NodeId node) const {
    assert(is_domination_computed(node));
    return m_tree.find(node)->second.idom;
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
  DominatorTree<NodeId> *m_tree;

  void find_dominated_by(Graph &graph, const NodeId &dominator) {
    std::unordered_set<NodeId> path;

    auto visitor = [dominator, &path](const NodeId &node) {
      if (node == dominator) {
        return false;
      }
      path.insert(node);
      return true;
    };

    visit_dfs_conditional(graph, m_entry, visitor);

    path.insert(dominator);
    for (auto &&node : m_all_nodes) {
      if (path.find(node) == path.end()) {
        auto res = m_tree->set_domination(dominator, node);
        assert(res && "Try to add invalid relation in Dom Tree");
      }
    }
  }

  NodeId find_immediate_dom(const NodeId &node) {
    assert(m_tree->is_domination_computed(node));
    if (node == m_entry) {
      return node;
    }
    NodeId imm_dom = *m_tree->dominated_by_begin(node);
    std::for_each(m_tree->dominated_by_begin(node), m_tree->dominated_by_end(node),
                  [this, &imm_dom](const NodeId &dom) {
                    if (m_tree->is_dominator_of(imm_dom, dom)) {
                      imm_dom = dom;
                    }
                  });
    return imm_dom;
  }

  void build(Graph &graph, const NodeId &entry, DominatorTree<NodeId> &tree) {
    m_entry = entry;
    m_tree = &tree;
    for (auto &&node : m_all_nodes) {
      find_dominated_by(graph, node);
    }
    for (auto &&node : m_all_nodes) {
      auto idom = find_immediate_dom(node);
      m_tree->set_immediate_dom(node, idom);
    }
  }

public:
  [[nodiscard]] auto build_tree(Graph &graph, const NodeId &entry) {
    DominatorTree<NodeId> tree;
    m_all_nodes.clear();
    auto nodes_inserter = std::back_inserter(m_all_nodes);
    visit_dfs(graph, entry, [&nodes_inserter](NodeId node) { *nodes_inserter = node; });
    build(graph, entry, tree);
    return tree;
  }

  template <typename NodeIt>
  [[nodiscard]] auto build_tree(Graph &graph, const NodeId &entry, NodeIt begin, NodeIt end) {
    DominatorTree<NodeId> tree;
    m_all_nodes.clear();
    std::copy(begin, end, std::back_inserter(m_all_nodes));
    build(graph, entry, tree);
    return tree;
  }
};

} // namespace koda