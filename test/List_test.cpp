#include <DataStructures/List.hpp>

#include <gtest/gtest.h>

namespace koda {

struct TestNode : public IntrusiveListNode<TestNode> {
  int m_id;

  TestNode(int id) : IntrusiveListNode(), m_id(id) {};
  virtual ~TestNode() = default;
};

std::vector<TestNode> makeNodes(size_t sz) {
  std::vector<TestNode> storage;
  for (size_t i = 1; i <= sz; i++) {
    storage.emplace_back(i);
  }
  return storage;
}

TEST(ListNodeTests, listInsertBack) {
  std::vector<TestNode> storage = makeNodes(10);
  IntrusiveList<TestNode> list;
  for (TestNode &node : storage) {
    list.insertBack(&node);
    ASSERT_EQ(list.getHead(), &storage[0]);
    ASSERT_EQ(list.getTail(), &node);
  }

  ASSERT_EQ(&storage[0], list.getHead());

  size_t list_size = 0;
  list.foreach ([&list_size](const TestNode &node) { list_size++; });

  ASSERT_EQ(list_size, storage.size());
}

TEST(ListNodeTests, listInsertFront) {
  std::vector<TestNode> storage = makeNodes(10);
  IntrusiveList<TestNode> list;
  for (TestNode &node : storage) {
    ASSERT_NE(&node, nullptr);
    list.insertFront(&node);
    ASSERT_EQ(list.getTail(), &storage[0]);
    ASSERT_EQ(list.getHead(), &node);
  }

  ASSERT_EQ(&storage[0], list.getTail());

  size_t list_size = 0;
  list.foreach ([&list_size](const TestNode &node) { list_size++; });

  ASSERT_EQ(list_size, storage.size());
}

} // namespace koda