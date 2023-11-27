#include <algorithm>
#include <set>
#include <vector>

namespace koda {

template <typename Graph> struct GraphTraits {
  using NodeId = typename Graph::NodeId;
  using PredIterator = typename Graph::PredIterator;
  using SuccIterator = typename Graph::SuccIterator;

  static PredIterator predBegin(Graph &owner, NodeId node) {
    return Graph::predBegin(owner, node);
  }
  static PredIterator predEnd(Graph &owner, NodeId node) {
    return Graph::predEnd(owner, node);
  }

  static SuccIterator succBegin(Graph &owner, NodeId node) {
    return Graph::succBegin(owner, node);
  }
  static SuccIterator succEnd(Graph &owner, NodeId node) {
    return Graph::succEnd(owner, node);
  }
};

template <typename Graph, typename Visitor>
void visit_dfs(Graph &graph, typename GraphTraits<Graph>::NodeId entry, Visitor visitor) {
  using Traits = GraphTraits<Graph>;

  std::vector<typename Traits::NodeId> worklist;
  std::set<typename Traits::NodeId> visited;

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
                      *worklist_inserter =  node;
                  });
  }
}

} // namespace koda