#include <DataStructures/List.hpp>

#include <gtest/gtest.h>

namespace koda {

struct TestNode : public IntrusiveListNode {

  int m_id;

  TestNode(int id) : IntrusiveListNode(), m_id(id){};
  virtual ~TestNode() = default;

  TestNode *get_next() const { return static_cast<TestNode *>(IntrusiveListNode::get_next()); }
  TestNode *get_prev() const { return static_cast<TestNode *>(IntrusiveListNode::get_prev()); }
};

std::vector<TestNode> makeNodes(size_t sz) {
  std::vector<TestNode> storage;
  for (size_t i = 1; i <= sz; i++) {
    storage.emplace_back(i);
  }
  return storage;
}

TEST(ListTests, list_insert_tail) {
  IntrusiveList<TestNode> list;
  std::vector<TestNode> storage = makeNodes(10);

  ASSERT_TRUE(list.empty());

  for (size_t i = 0; i < storage.size(); ++i) {
    TestNode &node = storage[i];
    TestNode *old_tail = list.get_tail();
    list.insert_tail(node);

    ASSERT_FALSE(list.empty());

    ASSERT_EQ(node.get_prev(), old_tail);
    ASSERT_FALSE(node.has_next());
    if (i == 0) {
      ASSERT_FALSE(node.has_prev());
    } else {
      ASSERT_TRUE(node.has_prev());
      ASSERT_TRUE(old_tail->has_next());
      ASSERT_EQ(old_tail->get_next(), &node);
    }

    ASSERT_EQ(list.get_head(), &storage[0]);
    ASSERT_EQ(list.get_tail(), &node);
  }

  ASSERT_EQ(&storage[0], list.get_head());

  size_t list_size = 0;
  for ([[maybe_unused]] auto &&node : list) {
    list_size++;
  }
  ASSERT_EQ(list_size, storage.size());
}

TEST(ListTests, list_insert_head) {
  std::vector<TestNode> storage = makeNodes(10);
  IntrusiveList<TestNode> list;

  ASSERT_TRUE(list.empty());

  for (size_t i = 0; i < storage.size(); ++i) {
    TestNode *node = &storage[i];
    TestNode *old_head = list.get_head();
    list.insert_head(node);

    ASSERT_FALSE(list.empty());

    ASSERT_EQ(node->get_next(), old_head);
    ASSERT_FALSE(node->has_prev());
    if (i == 0) {
      ASSERT_FALSE(node->has_next());
    } else {
      ASSERT_TRUE(node->has_next());
      ASSERT_TRUE(old_head->has_prev());
      ASSERT_EQ(old_head->get_prev(), node);
    }

    ASSERT_EQ(list.get_tail(), &storage[0]);
    ASSERT_EQ(list.get_head(), node);
  }

  ASSERT_EQ(&storage[0], list.get_tail());

  size_t list_size = 0;
  for ([[maybe_unused]] auto &&node : list) {
    list_size++;
  }

  ASSERT_EQ(list_size, storage.size());
}

TEST(ListTests, insert_after) {
  auto storage = makeNodes(3);
  IntrusiveList<TestNode> list;

  list.insert_tail(storage[0]);
  ASSERT_EQ(list.get_head()->m_id, storage[0].m_id);

  list.insert_after(*list.get_head(), storage[1]);
  ASSERT_EQ(list.get_tail()->m_id, storage[1].m_id);
  ASSERT_EQ(list.get_head()->get_next()->m_id, storage[1].m_id);
  ASSERT_EQ(list.get_head()->get_next(), list.get_tail());
  ASSERT_EQ(list.get_tail()->get_prev(), list.get_head());

  list.insert_after(list.get_head(), &storage[2]);
  const auto id = storage[2].m_id;
  ASSERT_EQ(list.get_head()->get_next()->m_id, id);
  ASSERT_EQ(list.get_tail()->get_prev()->m_id, id);

  std::vector<int> order;
  std::vector<int> gold_order = {1, 3, 2};

  for (auto &&node : list) {
    order.push_back(node.m_id);
  }

  ASSERT_EQ(gold_order.size(), order.size());
  for (size_t i = 0; i < gold_order.size(); i++) {
    ASSERT_EQ(gold_order[i], order[i]);
  }
}

TEST(ListTests, insert_before) {
  auto storage = makeNodes(3);
  IntrusiveList<TestNode> list;

  list.insert_head(storage[0]);
  ASSERT_EQ(list.get_tail()->m_id, storage[0].m_id);

  list.insert_before(*list.get_head(), storage[1]);
  ASSERT_EQ(list.get_head()->m_id, storage[1].m_id);
  ASSERT_EQ(list.get_tail()->get_prev()->m_id, storage[1].m_id);
  ASSERT_EQ(list.get_head()->get_next(), list.get_tail());
  ASSERT_EQ(list.get_tail()->get_prev(), list.get_head());

  list.insert_before(list.get_tail(), &storage[2]);
  const auto id = storage[2].m_id;
  ASSERT_EQ(list.get_head()->get_next()->m_id, id);
  ASSERT_EQ(list.get_tail()->get_prev()->m_id, id);

  std::vector<int> order;
  std::vector<int> gold_order = {2, 3, 1};

  for (auto &&node : list) {
    order.push_back(node.m_id);
  }

  ASSERT_EQ(gold_order.size(), order.size());
  for (size_t i = 0; i < gold_order.size(); i++) {
    ASSERT_EQ(gold_order[i], order[i]);
  }
}

TEST(ListTests, remove) {
  std::vector<TestNode> storage = makeNodes(3);
  IntrusiveList<TestNode> list;
  for (auto &&node : storage) {
    list.insert_tail(node);
  }
  TestNode *removed_node = list.get_head()->get_next();
  list.remove(removed_node);
  ASSERT_EQ(list.get_head()->get_next(), list.get_tail());
  ASSERT_EQ(list.get_tail()->get_prev(), list.get_head());

  // Covers removeTail()
  list.remove(list.get_tail());
  ASSERT_EQ(list.get_head(), list.get_tail());

  // Covers removeHead()
  list.remove(list.get_head());
  ASSERT_TRUE(list.empty());
}

} // namespace koda