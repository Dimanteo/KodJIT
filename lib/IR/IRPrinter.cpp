#include "IR/IRPrinter.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/ProgramGraph.hpp"
#include <IR/IRPrinter.hpp>

#include <algorithm>

namespace koda {

void IRPrinter::printProgGraph(ProgramGraph &graph) {
  m_out_stream << "digraph G {\n";

  graph.for_each_block([this](BasicBlock &bb) {
    auto id = bb.getID();
    m_out_stream << id << "[shape=record,label=\"";
    printBlock(bb);
    m_out_stream << "\"];\n";
    bb.for_each_succ(
        [id, this](const BasicBlock &succ) { m_out_stream << id << " -> " << succ.getID() << ";\n"; });
  });

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