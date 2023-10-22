#include "IR/IROperand.hpp"
#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <gtest/gtest.h>

namespace koda {

TEST(IRTests, empty_prog_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = builder.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);
}

TEST(IRTests, add_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = builder.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);

  auto param = builder.appendProgParam(OperandType::INTEGER);
  builder.createIAdd(builder.createIntConstant(42), param);
}

TEST(IRTests, factorial) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = builder.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);
}

} // namespace koda