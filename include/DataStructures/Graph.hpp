#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include <ostream>

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

template <typename Graph> struct PrintableGraphTraits {
  using BaseTraits = GraphTraits<Graph>;
  using NodeId = typename BaseTraits::NodeId;
  static std::string nodeToString(Graph &graph, NodeId node) { return Graph::nodeToString(graph, node); }
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

template <typename Graph>
void print_dot(Graph &graph, typename GraphTraits<Graph>::NodeId entry, std::ostream &out_str) {
  using Traits = PrintableGraphTraits<Graph>;
  using BaseTraits = typename Traits::BaseTraits;

  out_str << "digraph G {\n";
  auto print_visitor = [&out_str, &graph](typename Traits::NodeId node) {
    out_str << Traits::nodeToString(graph, node) << "\n";

    auto print_succ = [node, &out_str](typename Traits::NodeId succ) {
      out_str << node << " -> " << succ << "\n";
    };
    std::for_each(BaseTraits::succBegin(graph, node), BaseTraits::succEnd(graph, node), print_succ);
  };
  visit_dfs(graph, entry, print_visitor);

  out_str << "}";
}

} // namespace koda