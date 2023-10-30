#pragma once

#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>
#include <IR/ProgramGraph.hpp>

#include <ostream>
namespace koda {

class IRPrinter final {

  std::ostream &m_out_stream;

public:
  IRPrinter(std::ostream &out_stream) : m_out_stream(out_stream) {}

  void printProgGraph(const ProgramGraph &graph);

  void printBlock(BasicBlock *bb);
};

} // namespace koda