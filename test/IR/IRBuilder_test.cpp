#include "IR/IROperand.hpp"
#include "IR/IRPrinter.hpp"
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

  std::ofstream dot_log("br.dot", std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.printProgGraph(graph);
  dot_log.close();
}

TEST(IRTests, cond_br_test) {
  ProgramGraph prog;
  IRBuilder builder(prog);

  BasicBlock *entry = prog.createBasicBlock();
  builder.setEntryPoint(entry);
  builder.setInsertPoint(entry);

  BasicBlock *false_bb = prog.createBasicBlock();
  BasicBlock *true_bb = prog.createBasicBlock();
  BasicBlock *epilogue = prog.createBasicBlock();

  auto par_lhs = prog.createParam(OperandType::INTEGER);
  auto par_rhs = prog.createParam(OperandType::INTEGER);

  auto lhs = builder.createParamLoad(par_lhs);
  auto rhs = builder.createParamLoad(par_rhs);

  builder.createConditionalBranch(CMP_EQ, false_bb, true_bb, lhs, rhs);

  builder.setInsertPoint(false_bb);
  auto false_val = builder.createIntConstant(2);
  builder.createBranch(epilogue);

  builder.setInsertPoint(true_bb);
  auto true_val = builder.createIntConstant(3);
  builder.createBranch(epilogue);

  builder.setInsertPoint(epilogue);
  auto phi = builder.createPHI(OperandType::INTEGER);
  phi->addOption(false_bb, false_val);
  phi->addOption(true_bb, true_val);
  builder.createIMul(builder.createIntConstant(5), phi);

  std::ofstream dot_log("cond_br.dot", std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.printProgGraph(prog);
  dot_log.close();
}

} // namespace koda