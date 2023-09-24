#include <List/List.hpp>
#include <cassert>

namespace koda {

void IntrusiveListNode::insertAfter(NodePtr insertionPoint) {
  assert(insertionPoint != NIL_NODE() && "Trying to insert after NIL node");

  auto next = insertionPoint->getNext();
  setNext(next);

  if (next != NIL_NODE()) {
    next->setPrev(this);
  }

  insertionPoint->setNext(this);
  setPrev(insertionPoint);
}

void IntrusiveListNode::insertBefore(NodePtr insertionPoint) {
  assert(insertionPoint != NIL_NODE() && "Trying to insert before NIL node");

  auto prev = insertionPoint->getPrev();
  setPrev(prev);

  if (prev != NIL_NODE()) {
    prev->setNext(this);
  }

  insertionPoint->setPrev(this);
  setNext(insertionPoint);
}

IntrusiveListNode::NodePtr IntrusiveListNode::remove() {
  if (!isFirst()) {
    auto prev = getPrev();
    prev->setNext(getNext());
  }
  if (isLast()) {
    return NIL_NODE();
  }
  auto next = getNext();
  next->setPrev(getPrev());

  setPrev(NIL_NODE());
  setNext(NIL_NODE());
  return next;
}

} // namespace koda
