#pragma once

#include <stddef.h>
#include <vector>

namespace koda {

// Base node of intrusive linked list
//
class IntrusiveListNode {
public:
  using NodePtr = IntrusiveListNode *;
  constexpr static NodePtr NIL_NODE() { return nullptr; };

private:
  IntrusiveListNode *m_prev = nullptr;
  IntrusiveListNode *m_next = nullptr;

  void setNext(NodePtr next) { m_next = next; }

  void setPrev(NodePtr prev) { m_prev = prev; }

public:
  virtual ~IntrusiveListNode() = default;

  NodePtr getNext() const { return m_next; }

  template <class DerivedT = IntrusiveListNode> DerivedT *getNext() const {
    return static_cast<DerivedT *>(m_next);
  }

  template <class DerivedT = IntrusiveListNode> DerivedT *getPrev() const {
    return static_cast<DerivedT *>(m_prev);
  }

  bool isLast() const { return m_next == NIL_NODE(); }

  bool isFirst() const { return m_prev == NIL_NODE(); }

  void insertAfter(NodePtr insertionPoint);

  void insertAfter(IntrusiveListNode &insertionPoint) {
    insertAfter(&insertionPoint);
  }

  void insertBefore(NodePtr insertionPoint);

  void insertBefore(IntrusiveListNode &insertionPoint) {
    insertBefore(&insertionPoint);
  }

  // Remove this node from list. Returns pointer to the next node.
  //
  NodePtr remove();
};

} // namespace koda
