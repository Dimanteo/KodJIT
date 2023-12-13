#include "IR/IRPrinter.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/ProgramGraph.hpp"
#include <IR/IRPrinter.hpp>

#include <algorithm>

namespace koda {

void IRPrinter::print_prog_graph(ProgramGraph &graph) {
  assert(graph.get_entry() != nullptr && "Graph must have an entry point");
  m_out_stream << "digraph G {\n";

  auto bb_print = [this, &graph](ProgramGraph::BBPtr &bb) {
    m_out_stream << "\"" << ProgramGraph::node_to_string(graph, bb.get()) << "\" [shape=record,";
    if (bb.get() == graph.get_entry()) {
      m_out_stream << "color=\"red\",";
    }
    m_out_stream << "label=\"";
    print_block(*bb);
    m_out_stream << "\"];\n";
  };
  std::for_each(graph.begin(), graph.end(), bb_print);

  if (!graph.get_loop_tree().empty()) {
    size_t loop_cnt = 1;
    auto &&loop_tree = graph.get_loop_tree();
    for (auto loop : graph.get_loop_tree()) {
      m_out_stream << "subgraph cluster_" << loop_cnt++ << " {\n\tlabel = \"loop " << loop.first
                   << "\";\n\tcolor=blue\n";
      for (auto &&bb : loop_tree.get(loop.first)) {
        m_out_stream << "\t\"" << ProgramGraph::node_to_string(graph, bb) << "\";\n";
      }
      m_out_stream << "}\n";
    }
  }

  m_out_stream << GraphPrinter::make_dot_graph(graph, graph.get_entry());

  m_out_stream << "}";
}

void IRPrinter::print_block(BasicBlock &bb) {
  m_out_stream << "bb" << bb.get_id() << ":\\l ";
  for (const auto &inst : bb) {
    inst.dump(m_out_stream);
    m_out_stream << "\\l ";
  }
}

} // namespace koda