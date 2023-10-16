#include <DataStructures/List.hpp>

#include <gtest/gtest.h>
#include <algorithm>

namespace koda {

struct TestNode : public IntrusiveListNode<TestNode, IntrusiveList<TestNode>> {

  int m_id;

  TestNode(int id) : IntrusiveListNode(), m_id(id){};
  virtual ~TestNode() = default;
};

std::vector<TestNode> makeNodes(size_t sz) {
  std::vector<TestNode> storage;
  for (size_t i = 1; i <= sz; i++) {
    storage.emplace_back(i);
  }
  return storage;
}

TEST(ListTests, listinsertTail) {
  IntrusiveList<TestNode> list;
  std::vector<TestNode> storage = makeNodes(10);

  ASSERT_TRUE(list.empty());

  for (size_t i = 0; i < storage.size(); ++i) {
    TestNode &node = storage[i];
    TestNode *old_tail = list.getTail();
    list.insertTail(node);
    ASSERT_TRUE(node.isInContainer());

    ASSERT_FALSE(list.empty());

    ASSERT_EQ(node.getParent(), &list);
    ASSERT_EQ(node.getPrev(), old_tail);
    ASSERT_FALSE(node.hasNext());
    if (i == 0) {
      ASSERT_FALSE(node.hasPrev());
    } else {
      ASSERT_TRUE(node.hasPrev());
      ASSERT_TRUE(old_tail->hasNext());
      ASSERT_EQ(old_tail->getNext(), &node);
    }

    ASSERT_EQ(list.getHead(), &storage[0]);
    ASSERT_EQ(list.getTail(), &node);
  }

  ASSERT_EQ(&storage[0], list.getHead());

  size_t list_size = 0;
  list.foreach ([&list_size](const TestNode &node) { list_size++; });

  ASSERT_EQ(list_size, storage.size());
}

TEST(ListTests, listinsertHead) {
  std::vector<TestNode> storage = makeNodes(10);
  IntrusiveList<TestNode> list;

  ASSERT_TRUE(list.empty());

  for (size_t i = 0; i < storage.size(); ++i) {
    TestNode *node = &storage[i];
    TestNode *old_head = list.getHead();
    list.insertHead(node);
    ASSERT_TRUE(node->isInContainer());

    ASSERT_FALSE(list.empty());

    ASSERT_EQ(node->getParent(), &list);
    ASSERT_EQ(node->getNext(), old_head);
    ASSERT_FALSE(node->hasPrev());
    if (i == 0) {
      ASSERT_FALSE(node->hasNext());
    } else {
      ASSERT_TRUE(node->hasNext());
      ASSERT_TRUE(old_head->hasPrev());
      ASSERT_EQ(old_head->getPrev(), node);
    }

    ASSERT_EQ(list.getTail(), &storage[0]);
    ASSERT_EQ(list.getHead(), node);
  }

  ASSERT_EQ(&storage[0], list.getTail());

  size_t list_size = 0;
  list.foreach ([&list_size](const TestNode &node) { list_size++; });

  ASSERT_EQ(list_size, storage.size());
}

TEST(ListTests, insertAfter) {
  auto storage = makeNodes(3);
  IntrusiveList<TestNode> list;

  list.insertTail(storage[0]);
  ASSERT_TRUE(storage[0].isInContainer());
  ASSERT_EQ(list.getHead()->m_id, storage[0].m_id);

  list.insertAfter(*list.getHead(), storage[1]);
  ASSERT_EQ(list.getTail()->m_id, storage[1].m_id);
  ASSERT_EQ(list.getHead()->getNext()->m_id, storage[1].m_id);
  ASSERT_EQ(list.getHead()->getNext(), list.getTail());
  ASSERT_EQ(list.getTail()->getPrev(), list.getHead());

  list.insertAfter(list.getHead(), &storage[2]);
  const auto id = storage[2].m_id;
  ASSERT_EQ(list.getHead()->getNext()->m_id, id);
  ASSERT_EQ(list.getTail()->getPrev()->m_id, id);

  std::vector<int> order;
  std::vector<int> gold_order = {1, 3, 2};
  auto order_insit = std::back_inserter(order);
  list.foreach ([&order_insit](const TestNode &node) { order_insit = node.m_id; });

  ASSERT_EQ(gold_order.size(), order.size());
  for (size_t i = 0; i < gold_order.size(); i++) {
    ASSERT_EQ(gold_order[i], order[i]);
  }
}

TEST(ListTests, insertBefore) {
  auto storage = makeNodes(3);
  IntrusiveList<TestNode> list;

  list.insertHead(storage[0]);
  ASSERT_TRUE(storage[0].isInContainer());
  ASSERT_EQ(list.getTail()->m_id, storage[0].m_id);

  list.insertBefore(*list.getHead(), storage[1]);
  ASSERT_EQ(list.getHead()->m_id, storage[1].m_id);
  ASSERT_EQ(list.getTail()->getPrev()->m_id, storage[1].m_id);
  ASSERT_EQ(list.getHead()->getNext(), list.getTail());
  ASSERT_EQ(list.getTail()->getPrev(), list.getHead());

  list.insertBefore(list.getTail(), &storage[2]);
  const auto id = storage[2].m_id;
  ASSERT_EQ(list.getHead()->getNext()->m_id, id);
  ASSERT_EQ(list.getTail()->getPrev()->m_id, id);

  std::vector<int> order;
  std::vector<int> gold_order = {2, 3, 1};
  auto order_insit = std::back_inserter(order);
  list.foreach ([&order_insit](const TestNode &node) { order_insit = node.m_id; });

  ASSERT_EQ(gold_order.size(), order.size());
  for (size_t i = 0; i < gold_order.size(); i++) {
    ASSERT_EQ(gold_order[i], order[i]);
  }
}

TEST(ListTests, remove) {
  std::vector<TestNode> storage = makeNodes(3);
  IntrusiveList<TestNode> list;
  std::for_each(storage.begin(), storage.end(), [&list](TestNode &node){ list.insertTail(node); });
  TestNode *removed_node = list.getHead()->getNext();
  list.remove(removed_node);
  ASSERT_FALSE(removed_node->isInContainer());
  ASSERT_EQ(list.getHead()->getNext(), list.getTail());
  ASSERT_EQ(list.getTail()->getPrev(), list.getHead());
  
  // Covers removeTail()
  list.remove(list.getTail());
  ASSERT_EQ(list.getHead(), list.getTail());

  // Covers removeHead()
  list.remove(list.getHead());
  ASSERT_TRUE(list.empty());
}

} // namespace koda