#include "IR/IROperand.hpp"
#include "IR/IRPrinter.hpp"
#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <ios>

namespace koda {

namespace Tests {

void dumpCFG(const char *test_name, ProgramGraph &prog) {
  std::ofstream dot_log(test_name, std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.print_prog_graph(prog);
  dot_log.close();
}

void verify_inst_sequence(const std::vector<InstOpcode> &sequence, BasicBlock *bb) {
  size_t i = 0;
  for (auto inst_it = bb->begin(), inst_end = bb->end(); inst_it != inst_end; ++inst_it) {
    ASSERT_EQ(inst_it->get_opcode(), sequence[i]);
    i++;
  }
  ASSERT_EQ(i, sequence.size());
}

TEST(IRTests, empty_prog_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = graph.create_basic_block();
  builder.set_entry_point(bb);
  builder.set_insert_point(bb);
}

TEST(IRTests, remove_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);
  BasicBlock *bb = graph.create_basic_block();
  builder.set_entry_point(bb);
  builder.set_insert_point(bb);

  auto inst = builder.create_int_constant(42);
  bb->remove_instruction(inst);
  verify_inst_sequence({}, bb);
}

TEST(IRTests, add_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb = graph.create_basic_block();
  builder.set_entry_point(bb);
  builder.set_insert_point(bb);

  size_t par_idx = graph.create_param(OperandType::INTEGER);

  auto param = builder.create_param_load(par_idx);

  builder.create_iadd(builder.create_int_constant(42), param);

  verify_inst_sequence({INST_PARAM, INST_CONST, INST_ADD}, bb);
}

TEST(IRTests, branch_test) {
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb = graph.create_basic_block();
  BasicBlock *target = graph.create_basic_block();

  builder.set_entry_point(bb);
  builder.set_insert_point(bb);

  builder.create_branch(target);

  verify_inst_sequence({INST_BRANCH}, bb);
  verify_inst_sequence({}, target);

  dumpCFG("branch_test.dot", graph);
}

TEST(IRTests, cond_br_test) {
  ProgramGraph prog;
  IRBuilder builder(prog);

  BasicBlock *entry = prog.create_basic_block();
  builder.set_entry_point(entry);
  builder.set_insert_point(entry);

  BasicBlock *false_bb = prog.create_basic_block();
  BasicBlock *true_bb = prog.create_basic_block();
  BasicBlock *epilogue = prog.create_basic_block();

  auto par_lhs = prog.create_param(OperandType::INTEGER);
  auto par_rhs = prog.create_param(OperandType::INTEGER);

  auto lhs = builder.create_param_load(par_lhs);
  auto rhs = builder.create_param_load(par_rhs);

  builder.create_conditional_branch(CMP_EQ, false_bb, true_bb, lhs, rhs);

  builder.set_insert_point(false_bb);
  auto false_val = builder.create_int_constant(2);
  builder.create_branch(epilogue);

  builder.set_insert_point(true_bb);
  auto true_val = builder.create_int_constant(3);
  builder.create_branch(epilogue);

  builder.set_insert_point(epilogue);
  auto phi = builder.create_phi(OperandType::INTEGER);
  phi->add_option(false_bb, false_val);
  phi->add_option(true_bb, true_val);
  builder.create_imul(builder.create_int_constant(5), phi);

  verify_inst_sequence({INST_PARAM, INST_PARAM, INST_COND_BR}, entry);
  verify_inst_sequence({INST_CONST, INST_BRANCH}, false_bb);
  verify_inst_sequence({INST_CONST, INST_BRANCH}, true_bb);
  verify_inst_sequence({INST_PHI, INST_CONST, INST_MUL}, epilogue);

  dumpCFG("cond_br_test.dot", prog);
}

TEST(IRTests, factorial) {
  ProgramGraph prog;
  IRBuilder builder(prog);

  auto param_N = prog.create_param(OperandType::INTEGER);
  auto entry_bb = prog.create_basic_block();
  auto loop_head_bb = prog.create_basic_block();
  auto loop_bb = prog.create_basic_block();
  auto done_bb = prog.create_basic_block();

  builder.set_entry_point(entry_bb);
  builder.set_insert_point(entry_bb);

  auto res_init = builder.create_int_constant(1);
  auto iter_init = builder.create_int_constant(2);
  auto N = builder.create_param_load(param_N);

  builder.create_branch(loop_head_bb);
  builder.set_insert_point(loop_head_bb);

  auto iter = builder.create_phi(OperandType::INTEGER);
  auto res = builder.create_phi(OperandType::INTEGER);
  builder.create_conditional_branch(CmpFlag::CMP_G, loop_bb, done_bb, iter, N);

  builder.set_insert_point(loop_bb);

  auto res_loop = builder.create_imul(res, iter);

  auto iter_loop = builder.create_iadd(iter, builder.create_int_constant(1));
  builder.create_branch(loop_head_bb);

  iter->add_option(entry_bb, iter_init);
  iter->add_option(loop_bb, iter_loop);

  res->add_option(entry_bb, res_init);
  res->add_option(loop_bb, res_loop);

  dumpCFG("factorial.dot", prog);
}

} // namespace Tests

} // namespace koda