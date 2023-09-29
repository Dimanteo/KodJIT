#pragma once

#include <cassert>
#include <functional>
#include <stddef.h>
#include <type_traits>
#include <vector>

namespace koda {

// Base node of intrusive linked list
//
template <class T>
class IntrusiveListNode {
public:
  using NodePtr = T *;
  using Node = T;

  constexpr static NodePtr NIL_NODE() { return nullptr; };

  static bool isNIL(NodePtr node) { return node == NIL_NODE(); }

private:
  NodePtr m_prev = nullptr;
  NodePtr m_next = nullptr;

public:
  virtual ~IntrusiveListNode() = default;

  void setNext(NodePtr next) { m_next = next; }

  void setPrev(NodePtr prev) { m_prev = prev; }

  NodePtr getNext() const {
    return static_cast<NodePtr>(m_next);
  }

  NodePtr getPrev() const {
    return static_cast<NodePtr>(m_prev);
  }

  bool isLast() const { return isNIL(m_next); }

  bool isFirst() const { return isNIL(m_prev); }
};

// Container for intrusive list nodes. Node must be derived from IntrusiveNode.
//
template <class T> class IntrusiveList final {

  static_assert(std::is_base_of<IntrusiveListNode<T>, T>::value && "Node must be derived from IntrusiveNode");

public:
  using Node = T;
  using NodePtr = T *;

private:
  NodePtr m_head = nullptr;

  NodePtr m_tail = nullptr;

public:
  void insertBack(NodePtr node) {
    if (empty()) {
      assert(Node::isNIL(m_head) && "invalid value for head in empty list");
      assert(Node::isNIL(m_tail) && "invalid value for tail in empty list");

      m_head = node;
      m_tail = node;
      return;
    }
    assert(m_tail->isLast() && "tail must be last in list");

    m_tail->setNext(node);
    node->setPrev(node);
    m_tail = node;
  }

  void insertFront(NodePtr node) {
    assert(!Node::isNIL(node) && "Invalid node passed as argument");
    if (empty()) {
      assert(Node::isNIL(m_head) && "invalid value for head in empty list");
      assert(Node::isNIL(m_tail) && "invalid value for tail in empty list");

      m_head = node;
      m_tail = node;
      return;
    }
    assert(m_head->isFirst() && "head must be first in list");

    m_head->setPrev(node);
    node->setNext(m_head);
    m_head = node;
  }

  void insertAfter(NodePtr insertPoint, NodePtr node) {
    assert(!Node::isNIL(insertPoint) && "Invalid insertion point");
    assert(!Node::isNIL(node) && "Invalid node passed as argument");

    if (insertPoint->isLast()) {
      insertBack(node);
    }

    NodePtr next = insertPoint->getNext();
    next->setPrev(node);
    insertPoint->setNext(node);
    node->setNext(next);
    node->setPrev(insertPoint);
  }

  void insertBefore(NodePtr insertPoint, NodePtr node) {
    assert(!Node::isNIL(insertPoint) && "Invalid insertion point");
    assert(!Node::isNIL(node) && "Invalid node passed as argument");

    if (insertPoint->isFirst()) {
      insertFront(node);
    }

    NodePtr prev = insertPoint->getPrev();
    insertPoint->setPrev(node);
    prev->setNext(node);
    node->setNext(insertPoint);
    node->setPrev(prev);
  }

  void removeHead() {
    if (empty()) {
      return;
    }
    if (m_head->isLast()) {
      m_head = Node::NIL_NODE();
      m_tail = Node::NIL_NODE();
      return;
    }

    NodePtr new_head = m_head->getNext();
    m_head->setNext(Node::NIL_NODE());
    new_head->setPrev(Node::NIL_NODE());
    m_head = new_head;
  }

  void removeTail() {
    if (empty()) {
      return;
    }
    if (m_head->isFirst()) {
      m_head = Node::NIL_NODE();
      m_tail = Node::NIL_NODE();
      return;
    }

    NodePtr new_tail = m_head->getPrev();
    m_tail->setPrev(Node::NIL_NODE());
    new_tail->setNext(Node::NIL_NODE());
    m_tail = new_tail;
  }

  // Remove \p node from list.
  // \returns node immediately after removed one or NIL if node is last.
  NodePtr remove(NodePtr node) {
    assert(!Node::isNIL(node) && "Invalid node passed as argument");
    if (node->isFirst()) {
      removeHead();
      return m_head;
    }
    if (node->isLast()) {
      removeTail();
      return m_tail;
    }

    auto next = node->getNext();
    auto prev = node->getPrev();
    next->setPrev(prev);
    prev->setNext(next);

    node->setPrev(Node::NIL_NODE());
    node->setNext(Node::NIL_NODE());

    return next;
  }

  void foreach (std::function<void(const Node &node)> action) {
    NodePtr curr_node = getHead();
    while (!Node::isNIL(curr_node)) {
      action(*curr_node);
      curr_node = curr_node->getNext();
    }
  }

  NodePtr getHead() const { return m_head; }

  NodePtr getTail() const { return m_tail; }

  bool empty() const { return Node::isNIL(m_head); }
};

} // namespace koda
