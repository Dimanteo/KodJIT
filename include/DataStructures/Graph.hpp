#pragma once

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include <ostream>
#include <sstream>

namespace koda {

template <typename Graph> struct GraphTraits {
  using GraphType = typename std::remove_reference<Graph>::type;
  using NodeId = typename GraphType::NodeId;
  using PredIterator = typename GraphType::PredIterator;
  using SuccIterator = typename GraphType::SuccIterator;

  static PredIterator pred_begin(Graph &owner, NodeId &&node) {
    return GraphType::pred_begin(owner, node);
  }
  static PredIterator pred_end(Graph &owner, NodeId &&node) {
    return GraphType::pred_end(owner, node);
  }

  static SuccIterator succ_begin(Graph &owner, const NodeId &node) {
    return GraphType::succ_begin(owner, node);
  }
  static SuccIterator succ_end(Graph &owner, const NodeId &node) {
    return GraphType::succ_end(owner, node);
  }
};

template <typename Graph> struct PrintableGraphTraits {
  using BaseTraits = GraphTraits<Graph>;
  using NodeId = typename BaseTraits::NodeId;
  static std::string node_to_string(Graph &&graph, const NodeId &node) {
    return BaseTraits::GraphType::node_to_string(std::forward<Graph>(graph), node);
  }
};

template <typename Graph, typename Visitor, typename PostVisitor>
void visit_dfs_conditional(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry, Visitor &&visitor,
                           PostVisitor &&post_visitor) {
  using Traits = GraphTraits<Graph>;

  std::vector<typename Traits::NodeId> worklist;
  std::unordered_set<typename Traits::NodeId> visited;
  std::unordered_set<typename Traits::NodeId> exited;

  auto worklist_inserter = std::back_inserter(worklist);
  *worklist_inserter = entry;

  while (!worklist.empty()) {
    auto &&tail = worklist.back();

    bool visitor_res = false;
    if (visited.find(tail) == visited.end()) {
      visitor_res = visitor(tail);
      visited.insert(tail);
    }

    size_t worklist_sz = worklist.size();
    if (visitor_res) {
      std::for_each(Traits::succ_begin(graph, tail), Traits::succ_end(graph, tail),
                    [&worklist_inserter, &visited](typename Traits::NodeId node) {
                      if (visited.find(node) == visited.end()) {
                        *worklist_inserter = node;
                      }
                    });
    }

    if (worklist_sz == worklist.size()) {
      if (exited.find(tail) == exited.end()) {
        exited.insert(tail);
        post_visitor(tail);
      }
      worklist.pop_back();
    }
  }
}

template <typename Graph, typename Visitor>
void visit_dfs_conditional(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry,
                           Visitor &&visitor) {
  visit_dfs_conditional(std::forward<Graph>(graph), entry, std::forward<Visitor>(visitor),
                        [](const typename GraphTraits<Graph>::NodeId &dummy) { (void)dummy; });
}

template <typename Graph, typename Visitor, typename PostVisitor>
void visit_dfs(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry, Visitor &&visitor,
               PostVisitor &&post_visitor) {
  using NodeId = typename GraphTraits<Graph>::NodeId;
  auto forward_visitor = [visitor = std::forward<Visitor>(visitor)](NodeId node) {
    visitor(node);
    return true;
  };
  visit_dfs_conditional(std::forward<Graph>(graph), entry, forward_visitor,
                        std::forward<PostVisitor>(post_visitor));
}

template <typename Graph, typename Visitor>
void visit_dfs(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry, Visitor &&visitor) {
  using NodeId = typename GraphTraits<Graph>::NodeId;
  auto forward_visitor = [visitor = std::forward<Visitor>(visitor)](NodeId node) {
    visitor(node);
    return true;
  };
  visit_dfs_conditional(std::forward<Graph>(graph), entry, forward_visitor);
}

template <typename Graph, typename Visitor>
void visit_rpo(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry, Visitor &&visitor) {
  using Traits = GraphTraits<Graph>;
  using NodeId = typename Traits::NodeId;

  std::vector<typename Traits::NodeId> postorder;
  auto inserter = std::back_inserter(postorder);
  visit_dfs(
      std::forward<Graph>(graph), entry, [](const NodeId &dummy) { (void)dummy; },
      [&inserter](const NodeId &node) { *inserter = node; });

  std::for_each(postorder.rbegin(), postorder.rend(), std::forward<Visitor>(visitor));
}

namespace GraphPrinter {

template <typename Graph>
std::string make_dot_graph(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry) {
  using Traits = GraphTraits<Graph>;
  using PrintTraits = PrintableGraphTraits<Graph>;

  std::stringstream ss;

  auto print_visitor = [&ss, &graph](const typename Traits::NodeId &node) {
    auto print_succ = [node, &graph, &ss](const typename Traits::NodeId &succ) {
      ss << "\"" << PrintTraits::node_to_string(graph, node) << "\" -> \""
          << PrintTraits::node_to_string(graph, succ) << "\"\n";
    };
    std::for_each(Traits::succ_begin(graph, node), Traits::succ_end(graph, node), print_succ);
  };
  visit_dfs(std::forward<Graph>(graph), entry, print_visitor);

  return ss.str();
}

template<typename Graph>
void print_dot(Graph &&graph, const typename GraphTraits<Graph>::NodeId &entry, std::ostream &out_str) {
  out_str << "digraph G {\n";
  out_str << make_dot_graph(graph, entry);
  out_str << "}";
}

};

} // namespace koda