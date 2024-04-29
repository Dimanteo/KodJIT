#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>

namespace koda {

// Base node of intrusive linked list
//
class IntrusiveListNode {
  using Node = IntrusiveListNode;

public:
  constexpr static auto NIL_NODE() { return nullptr; };

  static bool is_nil(const Node *node) { return node == NIL_NODE(); }
  static bool is_nil(const Node &node) { return is_nil(&node); }

private:
  Node *m_prev = nullptr;
  Node *m_next = nullptr;

public:
  virtual ~IntrusiveListNode() = default;

  IntrusiveListNode() = default;

  void set_next(Node &next) { set_next(&next); }

  void set_prev(Node &prev) { set_prev(&prev); }

  void set_next(Node *next) { m_next = next; }

  void set_prev(Node *prev) { m_prev = prev; }

  Node *get_next() const { return m_next; }

  Node *get_prev() const { return m_prev; }

  bool has_next() const { return !is_nil(m_next); }

  bool has_prev() const { return !is_nil(m_prev); }
};

namespace detailList {

template <class InNode, bool IsReverse = false>
class IntrusiveListIterator final {

  static_assert(std::is_base_of<IntrusiveListNode, InNode>::value,
                "Node type must be derived from intrusive node");

public:
  using value_type = InNode;
  using pointer = value_type *;
  using reference = value_type &;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

private:
  pointer m_node_ptr = nullptr;

public:
  IntrusiveListIterator() = default;

  explicit IntrusiveListIterator(pointer node) : m_node_ptr(node) {}

  explicit IntrusiveListIterator(reference node) : m_node_ptr(&node) {}

  [[nodiscard]] reference operator*() const noexcept {
    return *static_cast<pointer>(m_node_ptr);
  }

  IntrusiveListIterator &operator++() noexcept {
    if constexpr (IsReverse) {
      m_node_ptr = static_cast<pointer>(m_node_ptr->get_prev());
    } else {
      m_node_ptr = static_cast<pointer>(m_node_ptr->get_next());
    }
    return *this;
  }

  IntrusiveListIterator &operator--() noexcept {
    if constexpr (IsReverse) {
      m_node_ptr = static_cast<pointer>(m_node_ptr->get_next());
    } else {
      m_node_ptr = static_cast<pointer>(m_node_ptr->get_prev());
    }
    return *this;
  }

  IntrusiveListIterator &operator++(int) noexcept {
    auto tmp = *this;
    ++*this;
    return tmp;
  }

  IntrusiveListIterator &operator--(int) noexcept {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  [[nodiscard]] pointer operator->() const noexcept {
    return static_cast<pointer>(m_node_ptr);
  }

  bool is_equal(IntrusiveListIterator &other) const noexcept {
    return m_node_ptr == other.m_node_ptr;
  }
};

template <typename InNode, bool IsReverse = false>
bool operator==(IntrusiveListIterator<InNode, IsReverse> lhs,
                IntrusiveListIterator<InNode, IsReverse> rhs) {
  return lhs.is_equal(rhs);
}

template <typename InNode, bool IsReverse = false>
bool operator!=(IntrusiveListIterator<InNode, IsReverse> lhs,
                IntrusiveListIterator<InNode, IsReverse> rhs) {
  return !(lhs == rhs);
}

} // namespace detailList

// Container for intrusive list nodes. InNode must be derived from
// IntrusiveListNode.
//
template <class InNode> class IntrusiveList final {
public:
  static_assert(std::is_base_of<IntrusiveListNode, InNode>::value,
                "Node type must be derived from intrusive node");

private:
  using BaseNode = IntrusiveListNode;

  BaseNode *m_head = nullptr;

  BaseNode *m_tail = nullptr;

  void set_head(BaseNode &node) { m_head = &node; }

  void set_head(BaseNode *node) { m_head = node; }

  void set_tail(BaseNode &node) { m_tail = &node; }

  void set_tail(BaseNode *node) { m_tail = node; }

  BaseNode *remove_impl(BaseNode &node) {
    assert(!InNode::is_nil(node) && "Invalid node passed as argument");

    if (!node.has_prev()) {
      remove_head();
      return m_head;
    }

    if (!node.has_next()) {
      remove_tail();
      return m_tail;
    }

    auto next = node.get_next();
    auto prev = node.get_prev();
    next->set_prev(prev);
    prev->set_next(next);

    node.set_prev(InNode::NIL_NODE());
    node.set_next(InNode::NIL_NODE());

    return next;
  }

public:
  using iterator = detailList::IntrusiveListIterator<InNode>;
  using reverse_iterator = detailList::IntrusiveListIterator<InNode, true>;
  using const_iterator =
      detailList::IntrusiveListIterator<std::add_const_t<InNode>>;
  using value_type = typename iterator::value_type;
  using pointer = typename iterator::pointer;
  using const_pointer = std::add_const_t<pointer>;
  using reference = typename iterator::reference;
  using const_reference = std::add_const_t<reference>;

  iterator begin() noexcept { return iterator{get_head()}; }
  iterator end() noexcept {
    return iterator{static_cast<pointer>(InNode::NIL_NODE())};
  }

  reverse_iterator rbegin() noexcept { return reverse_iterator{get_tail()}; }
  reverse_iterator rend() noexcept {
    return reverse_iterator{static_cast<pointer>(InNode::NIL_NODE())};
  }

  const_iterator cbegin() const noexcept { return const_iterator{get_head()}; }
  const_iterator cend() const noexcept {
    return const_iterator{static_cast<pointer>(InNode::NIL_NODE())};
  }

  void insert_tail(InNode *node) {
    assert(!InNode::is_nil(node) && "Invalid node passed as argument");
    insert_tail(*node);
  }

  void insert_tail(InNode &node) {
    if (empty()) {
      assert(InNode::is_nil(m_head) && "invalid value for head in empty list");
      assert(InNode::is_nil(m_tail) && "invalid value for tail in empty list");

      set_head(node);
      set_tail(node);
      node.set_next(InNode::NIL_NODE());
      node.set_prev(InNode::NIL_NODE());
      return;
    }
    assert(!m_tail->has_next() && "tail must be last in list");

    m_tail->set_next(node);
    node.set_prev(m_tail);
    set_tail(node);
  }

  void insert_head(InNode *node) {
    assert(!InNode::is_nil(node) && "Invalid node passed as argument");
    insert_head(*node);
  }

  void insert_head(InNode &node) {
    if (empty()) {
      assert(InNode::is_nil(m_head) && "invalid value for head in empty list");
      assert(InNode::is_nil(m_tail) && "invalid value for tail in empty list");

      set_head(node);
      set_tail(node);
      node.set_next(InNode::NIL_NODE());
      node.set_prev(InNode::NIL_NODE());
      return;
    }
    assert(!m_head->has_prev() && "head must be first in list");

    m_head->set_prev(node);
    node.set_next(m_head);
    set_head(node);
  }

  void insert_after(InNode *insertPoint, InNode *node) {
    assert(!InNode::is_nil(insertPoint) && "Invalid insertion point");
    assert(!InNode::is_nil(node) && "Invalid node passed as argument");
    insert_after(*insertPoint, *node);
  }

  void insert_after(InNode &insertPoint, InNode &node) {
    if (!insertPoint.has_next()) {
      insert_tail(node);
      return;
    }

    InNode *next = insertPoint.get_next();
    next->set_prev(node);
    insertPoint.set_next(node);
    node.set_next(next);
    node.set_prev(insertPoint);
  }

  void insert_before(InNode *insertPoint, InNode *node) {
    assert(!InNode::is_nil(insertPoint) && "Invalid insertion point");
    assert(!InNode::is_nil(node) && "Invalid node passed as argument");
    insert_before(*insertPoint, *node);
  }

  void insert_before(InNode &insertPoint, InNode &node) {
    if (!insertPoint.has_prev()) {
      insert_head(node);
      return;
    }

    InNode *prev = insertPoint.get_prev();
    insertPoint.set_prev(node);
    prev->set_next(node);
    node.set_next(insertPoint);
    node.set_prev(prev);
  }

  void remove_head() {
    if (empty()) {
      return;
    }

    if (!m_head->has_next()) {
      m_head->set_prev(InNode::NIL_NODE());
      m_head->set_next(InNode::NIL_NODE());
      set_head(InNode::NIL_NODE());
      set_tail(InNode::NIL_NODE());
      return;
    }

    BaseNode *new_head = m_head->get_next();
    m_head->set_prev(InNode::NIL_NODE());
    m_head->set_next(InNode::NIL_NODE());
    new_head->set_prev(InNode::NIL_NODE());
    set_head(new_head);
  }

  void remove_tail() {
    if (empty()) {
      return;
    }

    if (!m_tail->has_prev()) {
      m_tail->set_prev(InNode::NIL_NODE());
      m_tail->set_next(InNode::NIL_NODE());
      set_head(InNode::NIL_NODE());
      set_tail(InNode::NIL_NODE());
      return;
    }

    BaseNode *new_tail = m_tail->get_prev();
    m_tail->set_prev(InNode::NIL_NODE());
    m_tail->set_next(InNode::NIL_NODE());
    new_tail->set_next(InNode::NIL_NODE());
    set_tail(new_tail);
  }

  // Remove \p node from list.
  // \returns next node after removed one or NIL if node is last.
  InNode *remove(InNode *node) { return remove(*node); }

  // Remove \p node from list.
  // \returns next node after removed one or NIL if node is last.
  InNode *remove(InNode &node) {
    return reinterpret_cast<InNode *>(remove_impl(node));
  }

  InNode *get_head() const { return reinterpret_cast<InNode *>(m_head); }

  InNode *get_tail() const { return reinterpret_cast<InNode *>(m_tail); }

  bool empty() const { return InNode::is_nil(m_head); }
};

} // namespace koda
