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

  static bool isNIL(const Node *node) { return node == NIL_NODE(); }
  static bool isNIL(const Node &node) { return isNIL(&node); }

private:
  Node *m_prev = nullptr;
  Node *m_next = nullptr;

public:
  virtual ~IntrusiveListNode() = default;

  IntrusiveListNode() = default;

  void setNext(Node &next) { setNext(&next); }

  void setPrev(Node &prev) { setPrev(&prev); }

  void setNext(Node *next) { m_next = next; }

  void setPrev(Node *prev) { m_prev = prev; }

  Node *getNext() const { return m_next; }

  Node *getPrev() const { return m_prev; }

  bool hasNext() const { return !isNIL(m_next); }

  bool hasPrev() const { return !isNIL(m_prev); }
};

namespace detailList {

template <class InNode> class IntrusiveListIterator final {

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

  [[nodiscard]] reference operator*() const noexcept { return *static_cast<pointer>(m_node_ptr); }

  IntrusiveListIterator &operator++() noexcept {
    m_node_ptr = static_cast<pointer>(m_node_ptr->getNext());
    return *this;
  }

  IntrusiveListIterator &operator--() noexcept {
    m_node_ptr = m_node_ptr->getPrev();
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

  [[nodiscard]] pointer operator->() const noexcept { return static_cast<pointer>(m_node_ptr); }

  bool is_equal(IntrusiveListIterator &other) const noexcept { return m_node_ptr == other.m_node_ptr; }
};

template <typename InNode>
bool operator==(IntrusiveListIterator<InNode> lhs, IntrusiveListIterator<InNode> rhs) {
  return lhs.is_equal(rhs);
}

template <typename InNode>
bool operator!=(IntrusiveListIterator<InNode> lhs, IntrusiveListIterator<InNode> rhs) {
  return !(lhs == rhs);
}

} // namespace detailList

// Container for intrusive list nodes. InNode must be derived from IntrusiveListNode.
//
template <class InNode> class IntrusiveList final {
public:
  static_assert(std::is_base_of<IntrusiveListNode, InNode>::value,
                "Node type must be derived from intrusive node");

private:
  InNode *m_head = nullptr;

  InNode *m_tail = nullptr;

  void setHead(InNode &node) { m_head = &node; }

  void setHead(InNode *node) { m_head = node; }

  void setTail(InNode &node) { m_tail = &node; }

  void setTail(InNode *node) { m_tail = node; }

public:
  using iterator = detailList::IntrusiveListIterator<InNode>;
  using const_iterator = detailList::IntrusiveListIterator<std::add_const_t<InNode>>;
  using value_type = typename iterator::value_type;
  using pointer = typename iterator::pointer;
  using const_pointer = std::add_const_t<pointer>;
  using reference = typename iterator::reference;
  using const_reference = std::add_const_t<reference>;

  iterator begin() noexcept { return iterator{getHead()}; }
  iterator end() noexcept { return iterator{static_cast<pointer>(InNode::NIL_NODE())}; }

  const_iterator cbegin() const noexcept { return iterator{getHead()}; }
  const_iterator cend() const noexcept { return iterator{static_cast<pointer>(InNode::NIL_NODE())}; }

  void insertTail(InNode *node) {
    assert(!InNode::isNIL(node) && "Invalid node passed as argument");
    insertTail(*node);
  }

  void insertTail(InNode &node) {
    if (empty()) {
      assert(InNode::isNIL(m_head) && "invalid value for head in empty list");
      assert(InNode::isNIL(m_tail) && "invalid value for tail in empty list");

      setHead(node);
      setTail(node);
      node.setNext(InNode::NIL_NODE());
      node.setPrev(InNode::NIL_NODE());
      return;
    }
    assert(!m_tail->hasNext() && "tail must be last in list");

    m_tail->setNext(node);
    node.setPrev(m_tail);
    setTail(node);
  }

  void insertHead(InNode *node) {
    assert(!InNode::isNIL(node) && "Invalid node passed as argument");
    insertHead(*node);
  }

  void insertHead(InNode &node) {
    if (empty()) {
      assert(InNode::isNIL(m_head) && "invalid value for head in empty list");
      assert(InNode::isNIL(m_tail) && "invalid value for tail in empty list");

      setHead(node);
      setTail(node);
      node.setNext(InNode::NIL_NODE());
      node.setPrev(InNode::NIL_NODE());
      return;
    }
    assert(!m_head->hasPrev() && "head must be first in list");

    m_head->setPrev(node);
    node.setNext(m_head);
    setHead(node);
  }

  void insertAfter(InNode *insertPoint, InNode *node) {
    assert(!InNode::isNIL(insertPoint) && "Invalid insertion point");
    assert(!InNode::isNIL(node) && "Invalid node passed as argument");
    insertAfter(*insertPoint, *node);
  }

  void insertAfter(InNode &insertPoint, InNode &node) {
    if (!insertPoint.hasNext()) {
      insertTail(node);
      return;
    }

    InNode *next = insertPoint.getNext();
    next->setPrev(node);
    insertPoint.setNext(node);
    node.setNext(next);
    node.setPrev(insertPoint);
  }

  void insertBefore(InNode *insertPoint, InNode *node) {
    assert(!InNode::isNIL(insertPoint) && "Invalid insertion point");
    assert(!InNode::isNIL(node) && "Invalid node passed as argument");
    insertBefore(*insertPoint, *node);
  }

  void insertBefore(InNode &insertPoint, InNode &node) {
    if (!insertPoint.hasPrev()) {
      insertHead(node);
      return;
    }

    InNode *prev = insertPoint.getPrev();
    insertPoint.setPrev(node);
    prev->setNext(node);
    node.setNext(insertPoint);
    node.setPrev(prev);
  }

  void removeHead() {
    if (empty()) {
      return;
    }

    if (!m_head->hasNext()) {
      m_head->setPrev(InNode::NIL_NODE());
      m_head->setNext(InNode::NIL_NODE());
      setHead(InNode::NIL_NODE());
      setTail(InNode::NIL_NODE());
      return;
    }

    auto &&new_head = m_head->getNext();
    m_head->setPrev(InNode::NIL_NODE());
    m_head->setNext(InNode::NIL_NODE());
    new_head->setPrev(InNode::NIL_NODE());
    setHead(new_head);
  }

  void removeTail() {
    if (empty()) {
      return;
    }

    if (!m_tail->hasPrev()) {
      m_tail->setPrev(InNode::NIL_NODE());
      m_tail->setNext(InNode::NIL_NODE());
      setHead(InNode::NIL_NODE());
      setTail(InNode::NIL_NODE());
      return;
    }

    auto &&new_tail = m_tail->getPrev();
    m_tail->setPrev(InNode::NIL_NODE());
    m_tail->setNext(InNode::NIL_NODE());
    new_tail->setNext(InNode::NIL_NODE());
    setTail(new_tail);
  }

  // Remove \p node from list.
  // \returns next node after removed one or NIL if node is last.
  InNode *remove(InNode *node) { return remove(*node); }

  // Remove \p node from list.
  // \returns next node after removed one or NIL if node is last.
  InNode *remove(InNode &node) {
    assert(!InNode::isNIL(node) && "Invalid node passed as argument");

    if (!node.hasPrev()) {
      removeHead();
      return m_head;
    }

    if (!node.hasNext()) {
      removeTail();
      return m_tail;
    }

    auto next = node.getNext();
    auto prev = node.getPrev();
    next->setPrev(prev);
    prev->setNext(next);

    node.setPrev(InNode::NIL_NODE());
    node.setNext(InNode::NIL_NODE());

    return next;
  }

  InNode *getHead() const { return m_head; }

  InNode *getTail() const { return m_tail; }

  bool empty() const { return InNode::isNIL(m_head); }
};

} // namespace koda
