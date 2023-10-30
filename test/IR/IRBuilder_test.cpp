#include "IR/IROperand.hpp"
#include "IR/IRPrinter.hpp"
#include "IR/IRTypes.hpp"
#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <ios>

namespace koda {

TEST(IRTests, empty_prog_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = graph.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);
}

TEST(IRTests, add_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb = graph.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);

  size_t par_idx = graph.createParam(OperandType::INTEGER);

  auto param = builder.createParamLoad(par_idx);

  builder.createIAdd(builder.createIntConstant(42), param);
}

TEST(IRTests, branch_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb = graph.createBasicBlock();
  BasicBlock *target = graph.createBasicBlock();

  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);

  builder.createBranch(target);

  std::ofstream dot_log("graph.dot", std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.printProgGraph(graph);
  dot_log.close();
}

} // namespace koda