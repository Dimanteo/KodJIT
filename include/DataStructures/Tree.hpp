#pragma once

#include <DataStructures/Graph.hpp>

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

template <typename Key, typename Value> class Tree {

  struct Vertice {
    using Successors = std::vector<Key>;
    Value m_value;
    Key m_parent;
    Successors m_succ;

    explicit Vertice() = default;
    explicit Vertice(Key parent) : m_parent(parent) {}
  };

  Key m_root;

  std::unordered_map<Key, Vertice> m_tree;

public:
  using KeyType = Key;
  using ValueType = Value;
  using child_iterator = typename Vertice::Successors::iterator;
  using parent_iterator = typename std::unordered_map<Key, Vertice>::iterator;

  const Key m_none;

  Tree(Key none) : m_none(none) {}

  bool empty() const { return m_tree.empty(); }

  size_t size() const { return m_tree.size(); }

  bool contains(const Key &key) const { return key != m_none && m_tree.find(key) != m_tree.end(); }

  void insert(Key key) {
    assert(key != m_none && "Inserting invalid key in tree");
    if (key == m_none)
      return;
    m_tree[key] = Vertice(m_none);
  }

  void erase(Key key) {
    auto &&vertice = m_tree[key];
    for (auto &&child : vertice.m_succ) {
      m_tree[child].parent = m_none;
    }
    if (contains(vertice.m_parent)) {
      auto &&parent_vertice = m_tree[vertice.m_parent];
      auto it = std::find(parent_vertice.m_succ.begin(), parent_vertice.m_succ.end(), key);
      parent_vertice.m_succ.erase(it);
    } else if (key == m_root) {
      m_root = m_none;
    }
    m_tree.erase(key);
  }

  void clear() { m_tree.clear(); }

  bool set_root(Key key) {
    if (!contains(key))
      return false;
    if (contains(m_tree[key].m_parent))
      return false;
    m_root = key;
    return true;
  }

  Key get_root() const { return m_root; }

  void link(Key parent, Key child) {
    assert(contains(parent) && "vertice doesn't exist");
    assert(contains(child) && "vertice doesn't exist");

    auto &&parent_vert = m_tree[parent];
    auto &&child_vert = m_tree[child];

    parent_vert.m_succ.push_back(child);

    if (child_vert.m_parent != m_none) {
      unlink_parent(child);
    }
    child_vert.m_parent = parent;

    if (child == m_root) {
      Key new_root = parent;
      while (m_tree[new_root].m_parent != m_none) {
        new_root = m_tree[new_root].m_parent;
      }
      m_root = new_root;
    }
  }

  void unlink_parent(Key child) {
    if (!contains(child))
      return;
    auto &&child_vert = m_tree[child];
    Key parent = child_vert.m_parent;
    if (!contains(parent)) {
      return;
    }
    auto &succs = m_tree[parent].m_succ;
    auto it = std::find(succs.begin(), succs.end(), child);
    m_tree[parent].m_succ.erase(it);
    child_vert.m_parent = m_none;
  }

  const Value &get(Key key) const { return m_tree.find(key)->second.m_value; }
  Value &get(Key key) { return m_tree.find(key)->second.m_value; }
  Key get_parent(Key key) const { return m_tree.find(key)->second.m_parent; }
  Key get_child(Key key, size_t idx) const { return m_tree.find(key)->second.m_succ[idx]; }
  child_iterator children_begin(Key key) { return std::begin(m_tree[key].m_succ); }
  child_iterator children_end(Key key) { return std::end(m_tree[key].m_succ); }

  // Graph traits
  using NodeId = Key;
  using PredIterator = parent_iterator;
  using SuccIterator = child_iterator;

  PredIterator pred_begin(NodeId key) {
    auto par_key = m_tree.find(key)->second.m_parent;
    return m_tree.find(par_key);
  }
  PredIterator pred_end(NodeId key) {
    auto par_key = m_tree.find(key)->second.m_parent;
    auto par_it = m_tree.find(par_key);
    if (par_it == m_tree.end()) {
      return par_it;
    }
    return std::next(par_it);
  }
  static PredIterator pred_begin(Tree &owner, NodeId key) { return owner.pred_begin(key); }
  static PredIterator pred_end(Tree &owner, NodeId key) { return owner.pred_end(key); }

  static SuccIterator succ_begin(Tree &owner, NodeId key) { return owner.children_begin(key); }
  static SuccIterator succ_end(Tree &owner, NodeId key) { return owner.children_end(key); }

  // Printable graph traits
  static std::string node_to_string(Tree &tree, const NodeId &node) {
    (void)tree;
    return std::to_string(node);
  }
};