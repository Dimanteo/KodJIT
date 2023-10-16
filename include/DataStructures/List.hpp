#pragma once

#include <cassert>
#include <functional>
#include <stddef.h>
#include <type_traits>
#include <vector>

namespace koda {

// Base node of intrusive linked list
//
template <class Derived, class Container> class IntrusiveListNode {
  friend Container;

public:
  using Node = Derived;

  constexpr static Node *NIL_NODE() { return nullptr; };

  static bool isNIL(const Node *node) { return node == NIL_NODE(); }
  static bool isNIL(const Node &node) { return isNIL(&node); }

private:
  Node *m_prev = nullptr;
  Node *m_next = nullptr;
  Container *m_parent = nullptr;

  void setNext(Node &next) { setNext(&next); }

  void setPrev(Node &prev) { setPrev(&prev); }

  void setNext(Node *next) { m_next = next; }

  void setPrev(Node *prev) { m_prev = prev; }

  void setParent(Container &parent) { setParent(&parent); }

  void setParent(Container *parent) { m_parent = parent; }

public:
  virtual ~IntrusiveListNode() = default;

  IntrusiveListNode() = default;

  Derived *getNext() const { return static_cast<Derived *>(m_next); }

  Derived *getPrev() const { return static_cast<Derived *>(m_prev); }

  Container *getParent() const { return m_parent; }

  bool hasNext() const { return !isNIL(m_next); }

  bool hasPrev() const { return !isNIL(m_prev); }

  bool isInContainer() const { return m_parent != nullptr; }
};

// Container for intrusive list nodes. InNode must be derived from IntrusiveListNode.
//
template <class InNode> class IntrusiveList final {
public:
  using NodeBase = IntrusiveListNode<InNode, IntrusiveList>;

  static_assert(std::is_base_of<NodeBase, InNode>::value);

private:
  InNode *m_head = nullptr;

  InNode *m_tail = nullptr;

  void setHead(InNode &node) { m_head = &node; }

  void setHead(InNode *node) { m_head = node; }

  void setTail(InNode &node) { m_tail = &node; }

  void setTail(InNode *node) { m_tail = node; }

public:
  void insertTail(InNode *node) {
    assert(!InNode::isNIL(node) && "Invalid node passed as argument");
    insertTail(*node);
  }

  void insertTail(InNode &node) {
    assert(!node.isInContainer() && "Can't insert node from different list");

    node.setParent(this);

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
    assert(!node.isInContainer() && "Can't insert node from different list");

    node.setParent(this);

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

    assert(!node.isInContainer() && "Can't insert node from different list");

    node.setParent(this);

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

    assert(!node.isInContainer() && "Can't insert node from different list");

    node.setParent(this);

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

    m_head->setParent(nullptr);

    if (!m_head->hasNext()) {
      m_head->setPrev(InNode::NIL_NODE());
      m_head->setNext(InNode::NIL_NODE());
      setHead(InNode::NIL_NODE());
      setTail(InNode::NIL_NODE());
      return;
    }

    InNode *new_head = m_head->getNext();
    m_head->setPrev(InNode::NIL_NODE());
    m_head->setNext(InNode::NIL_NODE());
    new_head->setPrev(InNode::NIL_NODE());
    setHead(new_head);
  }

  void removeTail() {
    if (empty()) {
      return;
    }

    m_tail->setParent(nullptr);

    if (!m_tail->hasPrev()) {
      m_tail->setPrev(InNode::NIL_NODE());
      m_tail->setNext(InNode::NIL_NODE());
      setHead(InNode::NIL_NODE());
      setTail(InNode::NIL_NODE());
      return;
    }

    InNode *new_tail = m_tail->getPrev();
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
    assert(node.getParent() == this && "Can't remove node from different container");

    node.setParent(nullptr);

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

  void foreach (std::function<void(const InNode &node)> action) {
    InNode *curr_node = getHead();
    while (!InNode::isNIL(curr_node)) {
      action(*curr_node);
      curr_node = curr_node->getNext();
    }
  }

  InNode *getHead() const { return m_head; }

  InNode *getTail() const { return m_tail; }

  bool empty() const { return InNode::isNIL(m_head); }
};

} // namespace koda
