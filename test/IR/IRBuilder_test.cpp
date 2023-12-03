#include "IR/IROperand.hpp"
#include "IR/IRPrinter.hpp"
#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <ios>

namespace koda {

void dumpCFG(const char *test_name, ProgramGraph &prog) {
  std::ofstream dot_log(test_name, std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.printProgGraph(prog);
  dot_log.close();
}

void verify_inst_sequence(const std::vector<InstOpcode> &sequence, BasicBlock *bb) {
  size_t i = 0;
  for (auto inst_it = bb->begin(), inst_end = bb->end(); inst_it != inst_end; ++inst_it) {
    ASSERT_EQ(inst_it->getOpcode(), sequence[i]);
    i++;
  }
  ASSERT_EQ(i, sequence.size());
}

TEST(IRTests, empty_prog_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = graph.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);
}

TEST(IRTests, remove_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = graph.createBasicBlock();
  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);

  auto inst = builder.createIntConstant(42);
  bb->removeInstruction(inst);
  verify_inst_sequence({}, bb);
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

  verify_inst_sequence({INST_PARAM, INST_CONST, INST_ADD}, bb);
}

TEST(IRTests, branch_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb = graph.createBasicBlock();
  BasicBlock *target = graph.createBasicBlock();

  builder.setEntryPoint(bb);
  builder.setInsertPoint(bb);

  builder.createBranch(target);

  verify_inst_sequence({INST_BRANCH}, bb);
  verify_inst_sequence({}, target);

  dumpCFG("branch_test", graph);
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

  verify_inst_sequence({INST_PARAM, INST_PARAM, INST_COND_BR}, entry);
  verify_inst_sequence({INST_CONST, INST_BRANCH}, false_bb);
  verify_inst_sequence({INST_CONST, INST_BRANCH}, true_bb);
  verify_inst_sequence({INST_PHI, INST_CONST, INST_MUL}, epilogue);

  dumpCFG("cond_br_test", prog);
}

TEST(IRTests, factorial) {
  ProgramGraph prog;
  IRBuilder builder(prog);

  auto param_N = prog.createParam(OperandType::INTEGER);
  auto entry_bb = prog.createBasicBlock();
  auto loop_head_bb = prog.createBasicBlock();
  auto loop_bb = prog.createBasicBlock();
  auto done_bb = prog.createBasicBlock();

  builder.setEntryPoint(entry_bb);
  builder.setInsertPoint(entry_bb);

  auto res_init = builder.createIntConstant(1);
  auto iter_init = builder.createIntConstant(2);
  auto N = builder.createParamLoad(param_N);

  builder.createBranch(loop_head_bb);
  builder.setInsertPoint(loop_head_bb);

  auto iter = builder.createPHI(OperandType::INTEGER);
  auto res = builder.createPHI(OperandType::INTEGER);
  builder.createConditionalBranch(CmpFlag::CMP_G, loop_bb, done_bb, iter, N);

  builder.setInsertPoint(loop_bb);

  auto res_loop = builder.createIMul(res, iter);

  auto iter_loop = builder.createIAdd(iter, builder.createIntConstant(1));
  builder.createBranch(loop_head_bb);

  iter->addOption(entry_bb, iter_init);
  iter->addOption(loop_bb, iter_loop);

  res->addOption(entry_bb, res_init);
  res->addOption(loop_bb, res_loop);

  dumpCFG("factorial", prog);
}

} // namespace koda