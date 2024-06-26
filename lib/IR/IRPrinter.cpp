#include "IR/IRPrinter.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/ProgramGraph.hpp"
#include <IR/IRPrinter.hpp>

#include <algorithm>

namespace koda {

void IRPrinter::print_prog_graph(ProgramGraph &graph) {
  assert(graph.get_entry() != nullptr && "Graph must have an entry point");
  m_out_stream << "digraph G {\n";

  auto bb_print = [this, &graph](koda::BasicBlock &bb) {
    m_out_stream << "\"" << ProgramGraph::node_to_string(graph, &bb) << "\" [shape=record,";
    if (bb.get_id() == graph.get_entry()->get_id()) {
      m_out_stream << "color=\"red\",";
    }
    m_out_stream << "label=\"";
    print_block(bb);
    m_out_stream << "\"];\n";
  };
  std::for_each(graph.begin(), graph.end(), bb_print);

  m_out_stream << GraphPrinter::make_dot_graph(graph, graph.get_entry());

  m_out_stream << "}";
}

void IRPrinter::print_block(const BasicBlock &bb) {
  m_out_stream << "bb" << bb.get_id() << ":\\l ";
  for (const auto &inst : bb) {
    inst.dump(m_out_stream);
    m_out_stream << "\\l ";
  }
}

} // namespace koda