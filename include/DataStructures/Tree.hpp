#pragma once

#include <DataStructures/Graph.hpp>

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace koda {

template <typename Key> struct TreeParentIterator {
  using value_type = typename std::remove_reference<Key>::type;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using difference_type = size_t;
  using iterator_category = std::input_iterator_tag;

private:
  Key m_val;
  bool m_is_ended = false;

public:
  TreeParentIterator() = default;

  TreeParentIterator(const_reference val) : m_val(val), m_is_ended(false) {}

  const_reference operator*() const noexcept {
    assert(!m_is_ended);
    return m_val;
  }

  TreeParentIterator &operator++() noexcept {
    m_is_ended = true;
    return *this;
  }

  TreeParentIterator &operator++(int) noexcept {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  pointer operator->() const noexcept {
    assert(!m_is_ended);
    if (m_is_ended) {
      return nullptr;
    }
    return &m_val;
  }

  bool is_equal(const TreeParentIterator &other) const noexcept {
    return m_is_ended == other.m_is_ended && m_val == other.m_val;
  }
};

template <typename Key>
bool operator==(const TreeParentIterator<Key> &lhs, const TreeParentIterator<Key> &rhs) noexcept {
  return lhs.is_equal(rhs);
}

template <typename Key>
bool operator!=(const TreeParentIterator<Key> &lhs, const TreeParentIterator<Key> &rhs) noexcept {
  return !(lhs == rhs);
}

template <typename Key, typename Value> class Tree {
  class Vertice {
    friend Tree;

    using Successors = std::vector<Key>;

    Value m_value;

    Key m_parent;

    Successors m_succ;

  public:
    explicit Vertice() = default;

    explicit Vertice(Key parent) : m_parent(parent) {}

    Value &value() { return m_value; }

    const Value &value() const { return m_value; }
  };

  using VerticeMap = std::unordered_map<Key, Vertice>;

  Key m_root;

  VerticeMap m_tree;

public:
  using KeyType = Key;
  using ValueType = Value;
  using iterator = typename VerticeMap::iterator;
  using child_iterator = typename Vertice::Successors::iterator;

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
    assert(parent != child && "cant't link to self");
    assert(contains(parent) && "vertice doesn't exist");
    assert(contains(child) && "vertice doesn't exist");

    auto &&parent_vert = m_tree[parent];
    auto &&child_vert = m_tree[child];

    if (child_vert.m_parent != m_none) {
      unlink_parent(child);
    }

    parent_vert.m_succ.push_back(child);
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

  iterator begin() { return m_tree.begin(); }

  iterator end() { return m_tree.end(); }

  const Value &get(Key key) const { return m_tree.find(key)->second.m_value; }

  Value &get(Key key) { return m_tree.find(key)->second.m_value; }

  bool has_parent(Key key) const { return contains(get_parent(key)); }

  Key get_parent(Key key) const { return m_tree.find(key)->second.m_parent; }

  Key get_child(Key key, size_t idx) const { return m_tree.find(key)->second.m_succ[idx]; }

  child_iterator children_begin(Key key) { return m_tree[key].m_succ.begin(); }

  child_iterator children_end(Key key) { return m_tree[key].m_succ.end(); }

  // Graph traits
  using NodeId = Key;
  using PredIterator = TreeParentIterator<NodeId>;
  using SuccIterator = child_iterator;

  PredIterator pred_begin(const NodeId &key) {
    auto par_key = get_parent(key);
    if (par_key == m_none) {
      return pred_end(key);
    }
    return PredIterator(get_parent(key));
  }
  PredIterator pred_end(const NodeId &key) { return std::next(PredIterator(get_parent(key))); }
  static PredIterator pred_begin(Tree &owner, const NodeId &key) { return owner.pred_begin(key); }
  static PredIterator pred_end(Tree &owner, const NodeId &key) { return owner.pred_end(key); }

  static SuccIterator succ_begin(Tree &owner, const NodeId &key) { return owner.children_begin(key); }
  static SuccIterator succ_end(Tree &owner, const NodeId &key) { return owner.children_end(key); }

  // Printable graph traits
  static std::string node_to_string(Tree &tree, const NodeId &node) {
    (void)tree;
    return std::to_string(node);
  }
};

} // namespace koda