#include "IR/IRPrinter.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/ProgramGraph.hpp"
#include <IR/IRPrinter.hpp>

#include <algorithm>

namespace koda {

void IRPrinter::printProgGraph(const ProgramGraph &graph) {
  m_out_stream << "digraph G {";

  graph.for_each_block([this](const BasicBlock &bb) {
    auto id = bb.getID();
    bb.for_each_succ(
        [id, this](const BasicBlock &succ) { m_out_stream << id << " -> " << succ.getID() << ";\n"; });
  });

  m_out_stream << "}";
}

void IRPrinter::printBlock(BasicBlock *bb) {
  m_out_stream << "BB" << bb->getID() << ":\n";
  // TODO: For each instruction
}

} // namespace koda