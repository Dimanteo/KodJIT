#include <List/List.hpp>

#include <gtest/gtest.h>

namespace koda {

struct TestNode : public IntrusiveListNode {
  int m_id = 0;
  TestNode(int id) : m_id(id) {}
};

TEST(ListNodeTests, node_insert_after) {
  std::vector<TestNode> storage;

  // Create head
  storage.emplace_back(1);

  for (int i = 2; i < 8; ++i) {
    auto lastNode = storage.back();
    storage.emplace_back(i);
    storage.back().insertAfter(lastNode);
    ASSERT_FALSE(lastNode.isLast());
  }

  ASSERT_TRUE(storage.front().isFirst());
  ASSERT_TRUE(storage.back().isLast());

  TestNode *curr_node = &storage.front();
  int curr_id = 1;

  while (curr_node != IntrusiveListNode::NIL_NODE()) {
    ASSERT_EQ(curr_node->m_id, curr_id);
    curr_id++;
    curr_node = curr_node->getNext<TestNode>();
  }
}

// TODO: node_insert_before test
// TODO: remove test

} // namespace koda