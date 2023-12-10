#pragma once

#include <DataStructures/Graph.hpp>
#include <DataStructures/Tree.hpp>

#include <cassert>
#include <unordered_set>
#include <vector>

namespace koda {

template <typename NodeId> struct DominatorTree final : public Tree<NodeId, std::unordered_set<NodeId>> {

  using Tree<NodeId, std::unordered_set<NodeId>>::contains;
  using Tree<NodeId, std::unordered_set<NodeId>>::get;

  DominatorTree(NodeId poison) : Tree<NodeId, std::unordered_set<NodeId>>(poison) {}

  // Return false if given relation is invalid. In particular if @dominated already dominating @dominator
  bool set_domination(NodeId dominator, NodeId dominated) {
    if (contains(dominated) && is_dominator_of(dominated, dominator)) {
      return false;
    }
    if (is_dominator_of(dominated, dominator)) {
      return false;
    }
    get(dominated).insert(dominator);
    return true;
  }

  bool is_dominator_of(NodeId dominator, NodeId dominated) const {
    if (!contains(dominated) || !contains(dominator)) {
      return false;
    }
    auto &&doms = get(dominated);
    return doms.find(dominator) != doms.end();
  }
};

template <typename Graph> class DominatorTreeBuilder final {
  using Traits = GraphTraits<Graph>;
  using NodeId = typename Traits::NodeId;

  std::vector<NodeId> m_all_nodes;
  DominatorTree<NodeId> *m_tree;

  void find_dominated_by(Graph &graph, const NodeId &m_entry, const NodeId &dominator) {
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
      assert(node != m_tree->m_none && "None node not allowed in input set");
      if (path.find(node) == path.end()) {
        auto res = m_tree->set_domination(dominator, node);
        assert(res && "Try to add invalid relation in Dom Tree");
      }
    }
  }

  NodeId find_immediate_dom(const NodeId &node) {
    assert(m_tree->contains(node));
    NodeId imm_dom = node;
    std::for_each(m_tree->get(node).begin(), m_tree->get(node).end(), [this, &imm_dom, &node](const NodeId &dom) {
      if (imm_dom == node || m_tree->is_dominator_of(imm_dom, dom)) {
        imm_dom = dom;
      }
    });
    return imm_dom;
  }

  void build(Graph &graph, const NodeId &entry, DominatorTree<NodeId> &tree) {
    m_tree = &tree;
    for (auto &&node : m_all_nodes) {
      m_tree->insert(node);
    }
    for (auto &&node : m_all_nodes) {
      find_dominated_by(graph, entry, node);
    }
    for (auto &&node : m_all_nodes) {
      if (node == entry) {
        continue;
      }
      auto idom = find_immediate_dom(node);
      if (idom != node) {
        m_tree->link(idom, node);
      }
    }
    m_tree->set_root(entry);
  }

public:
  void build_tree(Graph &graph, const NodeId &entry, DominatorTree<NodeId> &tree) {
    m_all_nodes.clear();
    auto nodes_inserter = std::back_inserter(m_all_nodes);
    visit_dfs(graph, entry, [&nodes_inserter](NodeId node) { *nodes_inserter = node; });
    build(graph, entry, tree);
  }

  template <typename NodeIt>
  void build_tree(Graph &graph, const NodeId &entry, NodeIt begin, NodeIt end, DominatorTree<NodeId> &tree) {
    m_all_nodes.clear();
    std::copy(begin, end, std::back_inserter(m_all_nodes));
    build(graph, entry, tree);
  }
};

} // namespace koda