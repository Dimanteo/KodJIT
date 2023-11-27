#include <algorithm>
#include <unordered_set>
#include <vector>

namespace koda {

template <typename Graph> struct GraphTraits {
  using NodeId = typename Graph::NodeId;
  using PredIterator = typename Graph::PredIterator;
  using SuccIterator = typename Graph::SuccIterator;

  static PredIterator predBegin(Graph &owner, NodeId node) { return Graph::predBegin(owner, node); }
  static PredIterator predEnd(Graph &owner, NodeId node) { return Graph::predEnd(owner, node); }

  static SuccIterator succBegin(Graph &owner, NodeId node) { return Graph::succBegin(owner, node); }
  static SuccIterator succEnd(Graph &owner, NodeId node) { return Graph::succEnd(owner, node); }
};

template <typename Graph, typename Visitor>
void visit_dfs(Graph &graph, typename GraphTraits<Graph>::NodeId entry, Visitor visitor) {
  using Traits = GraphTraits<Graph>;

  std::vector<typename Traits::NodeId> worklist;
  std::unordered_set<typename Traits::NodeId> visited;

  auto worklist_inserter = std::back_inserter(worklist);
  *worklist_inserter = entry;

  while (!worklist.empty()) {
    typename Traits::NodeId tail = worklist.back();
    worklist.pop_back();

    visitor(tail);
    visited.insert(tail);

    std::for_each(Traits::succBegin(graph, tail), Traits::succEnd(graph, tail),
                  [&worklist_inserter, &visited](typename Traits::NodeId node) {
                    if (visited.find(node) == visited.end())
                      *worklist_inserter = node;
                  });
  }
}

template <typename Graph, typename Visitor>
void visit_rpo(Graph &graph, typename GraphTraits<Graph>::NodeId entry, Visitor visitor) {
  using Traits = GraphTraits<Graph>;

  std::vector<typename Traits::NodeId> postorder;
  std::vector<typename Traits::NodeId> worklist;
  std::unordered_set<typename Traits::NodeId> visited;

  auto post_inserter = std::back_inserter(postorder);
  auto worklist_inserter = std::back_inserter(worklist);
  *worklist_inserter = entry;

  while (!worklist.empty()) {
    typename Traits::NodeId tail = worklist.back();
    visited.insert(tail);

    size_t worklist_sz = worklist.size();
    std::for_each(Traits::succBegin(graph, tail), Traits::succEnd(graph, tail),
                  [&worklist_inserter, &visited](typename Traits::NodeId node) {
                    if (visited.find(node) == visited.end())
                      *worklist_inserter = node;
                  });
    if (worklist_sz == worklist.size()) {
      // No new nodes added to worklist. Means DFS exiting this node.
      worklist.pop_back();
      *post_inserter = tail;
    }
  }

  std::for_each(postorder.rbegin(), postorder.rend(), visitor);
}

} // namespace koda