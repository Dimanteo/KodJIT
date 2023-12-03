#include "IR/IRPrinter.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/ProgramGraph.hpp"
#include <IR/IRPrinter.hpp>

#include <algorithm>

namespace koda {

void IRPrinter::printProgGraph(ProgramGraph &graph) {
  m_out_stream << "digraph G {\n";

  auto bb_print = [this](ProgramGraph::BBPtr &bb) {
    m_out_stream << bb.get() << "[shape=record,label=\"";
    printBlock(*bb);
    m_out_stream << "\"];\n";
  };
  std::for_each(graph.begin(), graph.end(), bb_print);

  m_out_stream << GraphPrinter<ProgramGraph>::make_dot_graph(graph, graph.getEntry());

  m_out_stream << "}";
}

void IRPrinter::printBlock(BasicBlock &bb) {
  m_out_stream << "bb" << bb.getID() << ":\\l ";
  for (const auto &inst : bb) {
    inst.dump(m_out_stream);
    m_out_stream << "\\l ";
  }
}

} // namespace koda