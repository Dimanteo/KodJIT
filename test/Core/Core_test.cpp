#include <gtest/gtest.h>

#include "Core/Compiler.h"
#include "IR/IRBuilder.hpp"
#include "IR/IRPrinter.hpp"
#include <fstream>
#include <vector>

namespace koda {
namespace Tests {

void dump_graph(ProgramGraph &graph, std::string filename) {
  std::ofstream dot_log(filename + ".dot", std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.print_prog_graph(graph);
  dot_log.close();
}

void dump_loops(Compiler &comp, std::string filename) {
  std::ofstream dot_log(filename + ".dot", std::ios_base::out);
  dot_log << "digraph {\n";
  auto &&loop_tree = comp.get_or_create<LoopTreeAnalysis>(comp).get();
  for (auto &&loop_it : loop_tree) {
    dot_log << "\"" << loop_it.first << "\" [shape=record,label=\"";
    dot_log << "head " << loop_it.first << "\\l Blocks";
    for (auto &&bb : loop_it.second.value()) {
      dot_log << " " << bb->get_id();
    }
    dot_log << "\\l Latches";
    for (auto &&bb : loop_it.second.value().get_latches()) {
      dot_log << " " << bb->get_id();
    }
    dot_log << "\"];\n";
  }
  dot_log << GraphPrinter::make_dot_graph(loop_tree, loop_tree.get_root());
  dot_log << "}";
  dot_log.close();
}

void connect(BasicBlock *from, BasicBlock *to, IRBuilder &builder) {
  builder.set_insert_point(from);
  builder.create_branch(to);
}

void connect(BasicBlock *from, BasicBlock *left, BasicBlock *right,
             IRBuilder &builder) {
  builder.set_insert_point(from);
  auto dummy = builder.create_int_constant(10);
  builder.create_conditional_branch(CmpFlag::CMP_EQ, left, right, dummy, dummy);
}

#define MKBB(name) BasicBlock *bb##name = graph.create_basic_block()
#define EDGE(from, to) connect(bb##from, bb##to, builder)
#define COND(from, fsucc, tsucc)                                               \
  connect(bb##from, bb##fsucc, bb##tsucc, builder)

TEST(CoreTest, linear_order_test) {
  Compiler comp;
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  MKBB(1);
  MKBB(2);
  MKBB(3);
  MKBB(4);
  MKBB(5);
  MKBB(6);
  MKBB(7);
  MKBB(8);
  MKBB(9);
  MKBB(10);
  MKBB(11);
  MKBB(12);
  MKBB(13);
  MKBB(14);
  MKBB(15);
  builder.set_entry_point(bb0);
  EDGE(0, 2);
  COND(2, 4, 3);
  COND(4, 5, 3);
  EDGE(5, 11);
  COND(11, 12, 13);
  EDGE(12, 4);
  EDGE(13, 1);
  EDGE(3, 6);
  EDGE(6, 7);
  EDGE(7, 8);
  COND(8, 14, 9);
  EDGE(9, 10);
  EDGE(10, 6);
  EDGE(14, 15);
  EDGE(15, 3);

  dump_graph(graph, "LinearOrderTest");
  auto &&linear_order = comp.get_or_create<LinearOrder>(comp);
  const std::vector<int> ref{0, 2, 4, 5, 11, 12, 13, 1,
                             3, 6, 7, 8, 9,  10, 14, 15};
  ASSERT_EQ(std::distance(linear_order.begin(), linear_order.end()),
            ref.size());
  unsigned i = 0;
  for (auto id = linear_order.begin(); id != linear_order.end(); ++id) {
    std::cout << (*id)->get_id() << " ";
    ASSERT_EQ((*id)->get_id(), ref[i++]);
  }
  dump_loops(comp, "LinearOrderTestLoops");
}

TEST(CoreTest, liveness_test) {
  Compiler comp;
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  MKBB(1);
  MKBB(2);
  MKBB(3);
  MKBB(4);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto inst_c1 = builder.create_int_constant(1);
  auto inst_c10 = builder.create_int_constant(10);
  auto inst_c20 = builder.create_int_constant(20);
  bb0->set_uncond_successor(bb1);

  builder.set_insert_point(bb1);
  auto phi_c1_loop = builder.create_phi(INTEGER);
  auto phi_c10_loop = builder.create_phi(INTEGER);
  auto cmp = builder.create_isub(phi_c10_loop, inst_c1);
  builder.create_conditional_branch(CMP_NE, bb3, bb2, cmp, cmp);

  builder.set_insert_point(bb2);
  auto mul = builder.create_imul(phi_c1_loop, phi_c10_loop);
  auto sub = builder.create_isub(phi_c10_loop, inst_c1);
  bb2->set_uncond_successor(bb1);

  builder.set_insert_point(bb3);
  auto ret = builder.create_iadd(inst_c20, phi_c1_loop);
  builder.create_iadd(ret, ret);
  builder.create_branch(bb4);

  phi_c1_loop->add_option(bb0, inst_c1);
  phi_c1_loop->add_option(bb2, mul);
  phi_c10_loop->add_option(bb0, inst_c10);
  phi_c10_loop->add_option(bb2, sub);

  dump_graph(graph, "LivenessTest");
  auto &&liveness = comp.get_or_create<Liveness>(comp);
  auto &&lin_order = comp.get_or_create<LinearOrder>(comp);
  size_t lin_num = 0;
  // clang-format off
  std::vector<std::pair<size_t, size_t>> ref_ranges = {
    {2, 20},
    {4, 8},
    {6, 22},
    {8, 22},
    {8, 18},
    {10, 12},
    {0, 0},
    {16, 20},
    {18, 20},
    {22, 24},
    {0, 0},
    {0, 0}
  };
  // clang-format on
  for (auto &&bb : lin_order) {
    for (auto &&inst : *bb) {
      auto &&range = liveness.get_live_range(inst.get_id());
      ASSERT_EQ(ref_ranges[lin_num++], range);
    }
  }
}

TEST(CoreTest, regalloc_test) {
  Compiler comp(3);
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  MKBB(1);
  MKBB(2);
  MKBB(3);
  MKBB(4);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto inst_c1 = builder.create_int_constant(1);
  auto inst_c10 = builder.create_int_constant(10);
  auto inst_c20 = builder.create_int_constant(20);
  bb0->set_uncond_successor(bb1);

  builder.set_insert_point(bb1);
  auto phi_c1_loop = builder.create_phi(INTEGER);
  auto phi_c10_loop = builder.create_phi(INTEGER);
  auto cmp = builder.create_isub(phi_c10_loop, inst_c1);
  builder.create_conditional_branch(CMP_NE, bb3, bb2, cmp, cmp);

  builder.set_insert_point(bb2);
  auto mul = builder.create_imul(phi_c1_loop, phi_c10_loop);
  auto sub = builder.create_isub(phi_c10_loop, inst_c1);
  bb2->set_uncond_successor(bb1);

  builder.set_insert_point(bb3);
  auto ret = builder.create_iadd(inst_c20, phi_c1_loop);
  builder.create_iadd(ret, ret);
  builder.create_branch(bb4);

  phi_c1_loop->add_option(bb0, inst_c1);
  phi_c1_loop->add_option(bb2, mul);
  phi_c10_loop->add_option(bb0, inst_c10);
  phi_c10_loop->add_option(bb2, sub);

  dump_graph(graph, "RegallocTest");

  auto &&regalloc = comp.get_or_create<RegAlloc>(comp);

  std::vector<std::optional<RegAlloc::Location>> ref_locs(
      graph.get_instr_count());
#define REG(inst, reg) ref_locs[inst] = RegAlloc::Location{reg, false}
#define STK(inst, slot)                                                        \
  ref_locs[inst] = RegAlloc::Location { slot, true }
  REG(0, 0);
  REG(1, 1);
  STK(2, 1);
  STK(3, 0);
  REG(4, 1);
  REG(5, 2);
  ref_locs[6] = std::nullopt;
  REG(7, 2);
  REG(8, 1);
  REG(9, 1);
  ref_locs[10] = std::nullopt;
  ref_locs[11] = std::nullopt;
#undef REG
#undef STK
  for (auto &&bb : graph) {
    for (auto &&inst : bb) {
      auto location = regalloc.get_location(inst.get_id());
      ASSERT_EQ(location.has_value(), ref_locs[inst.get_id()].has_value());
      if (location.has_value()) {
        std::cout << "i" << inst.get_id() << " ";
        if (location->is_stack)
          std::cout << "s";
        else
          std::cout << "r";
        std::cout << location->location << "\n";
        ASSERT_EQ(location->is_stack, ref_locs[inst.get_id()]->is_stack);
        ASSERT_EQ(location->location, ref_locs[inst.get_id()]->location);
      }
    }
  }
}

TEST(CoreTest, and_fold) {
  Compiler comp;
  comp.register_pass<ConstantFolding>();
  comp.register_pass<RmUnused>();
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto lhs = builder.create_int_constant(7);
  auto rhs = builder.create_int_constant(2);
  auto res = builder.create_and(lhs, rhs);
  auto term = builder.create_ret(res);
  dump_graph(graph, "FoldAndTest0");
  comp.run_all_passes();
  dump_graph(graph, "FoldAndTest1");
  ASSERT_EQ(bb0->size(), 2);
  ASSERT_EQ(bb0->front().get_opcode(), INST_CONST);
  auto val = dynamic_cast<LoadConstant<int64_t> &>(bb0->front()).get_value();
  ASSERT_EQ(val, 7 & 2);
  ASSERT_EQ(&bb0->back(), term);
}

TEST(CoreTest, sub_fold) {
  Compiler comp;
  comp.register_pass<ConstantFolding>();
  comp.register_pass<RmUnused>();
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto lhs = builder.create_int_constant(7);
  auto rhs = builder.create_int_constant(2);
  auto res = builder.create_isub(lhs, rhs);
  auto term = builder.create_ret(res);
  dump_graph(graph, "FoldSubTest0");
  comp.run_all_passes();
  dump_graph(graph, "FoldSubTest1");
  ASSERT_EQ(bb0->size(), 2);
  ASSERT_EQ(bb0->front().get_opcode(), INST_CONST);
  auto val = dynamic_cast<LoadConstant<int64_t> &>(bb0->front()).get_value();
  ASSERT_EQ(val, 7 - 2);
  ASSERT_EQ(&bb0->back(), term);
}

TEST(CoreTest, shr_fold) {
  Compiler comp;
  comp.register_pass<ConstantFolding>();
  comp.register_pass<RmUnused>();
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto lhs = builder.create_int_constant(32);
  auto rhs = builder.create_int_constant(3);
  auto res = builder.create_shr(lhs, rhs);
  auto term = builder.create_ret(res);
  dump_graph(graph, "FoldShrTest0");
  comp.run_all_passes();
  dump_graph(graph, "FoldShrTest1");
  ASSERT_EQ(bb0->size(), 2);
  ASSERT_EQ(bb0->front().get_opcode(), INST_CONST);
  auto val = dynamic_cast<LoadConstant<int64_t> &>(bb0->front()).get_value();
  ASSERT_EQ(val, 32 >> 3);
  ASSERT_EQ(&bb0->back(), term);
}

TEST(CoreTest, cross_bb_fold) {
  Compiler comp;
  comp.register_pass<ConstantFolding>();
  comp.register_pass<RmUnused>();
  auto &&graph = comp.graph();
  IRBuilder builder(graph);
  MKBB(0);
  MKBB(1);
  MKBB(2);
  builder.set_entry_point(bb0);
  // bb0:
  // lhs = 10
  // rhs = 13
  // add_res = lhs + rhs
  // cmp_const = 25
  // if (add_res == 25) { goto bb2 } else { goto bb1 }
  builder.set_insert_point(bb0);
  auto lhs = builder.create_int_constant(10);
  auto rhs = builder.create_int_constant(13);
  auto add_res = builder.create_iadd(lhs, rhs);
  auto cmp_const = builder.create_int_constant(25);
  auto branch =
      builder.create_conditional_branch(CMP_EQ, bb1, bb2, add_res, cmp_const);
  // bb1:
  // ret lhs
  builder.set_insert_point(bb1);
  builder.create_ret(lhs);
  // bb2:
  // sub_res = add_res - cmp_const
  // ret sub_res
  builder.set_insert_point(bb2);
  auto sub_res = builder.create_isub(add_res, cmp_const);
  auto ret_inst = builder.create_ret(sub_res);

  dump_graph(graph, "FoldCrossBBTest0");
  comp.run_all_passes();
  dump_graph(graph, "FoldCrossBBTest1");

  ASSERT_EQ(bb0->size(), 4);
  auto folded_lhs = branch->get_lhs();
  ASSERT_EQ(folded_lhs->get_opcode(), INST_CONST);
  ASSERT_EQ(dynamic_cast<LoadConstant<int64_t> *>(folded_lhs)->get_value(), 23);

  ASSERT_EQ(bb2->size(), 2);
  auto folded_ret = ret_inst->get_input();
  ASSERT_EQ(folded_ret->get_opcode(), INST_CONST);
  ASSERT_EQ(dynamic_cast<LoadConstant<int64_t> *>(folded_ret)->get_value(), -2);
}

auto has_inst(BasicBlock &bb, InstOpcode opc) {
  return std::any_of(bb.begin(), bb.end(), [opc](const Instruction &inst) {
    return inst.get_opcode() == opc;
  });
};

TEST(CoreTest, peephole_and) {
  Compiler comp;
  comp.register_pass<Peephole>();
  comp.register_pass<RmUnused>();
  auto &&graph = comp.graph();
  graph.create_param(INTEGER);
  IRBuilder builder(graph);
  MKBB(0);
  MKBB(1);
  MKBB(2);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto var = builder.create_param_load(0);
  builder.create_and(var, var);
  builder.set_insert_point(bb1);
  auto var_copy = builder.create_and(var, builder.create_int_constant(~0ul));
  builder.set_insert_point(bb2);
  auto zero = builder.create_and(var_copy, builder.create_int_constant(0));
  builder.create_iadd(var_copy, zero);
  EDGE(0, 1);
  EDGE(1, 2);
  dump_graph(graph, "PeepAndTest0");
  comp.run_all_passes();
  dump_graph(graph, "PeepAndTest1");
  ASSERT_EQ(bb0->size(), 2);
  ASSERT_EQ(bb1->size(), 1);
  ASSERT_EQ(bb2->size(), 2);
  ASSERT_FALSE(has_inst(*bb0, INST_AND));
  ASSERT_FALSE(has_inst(*bb1, INST_AND));
  ASSERT_FALSE(has_inst(*bb2, INST_AND));
}

TEST(CoreTest, peephole_sub) {
  Compiler comp;
  comp.register_pass<Peephole>();
  comp.register_pass<RmUnused>();
  auto &&graph = comp.graph();
  graph.create_param(INTEGER);
  IRBuilder builder(graph);
  MKBB(0);
  builder.set_entry_point(bb0);
  builder.set_insert_point(bb0);
  auto var = builder.create_param_load(0);
  auto var_zero = builder.create_isub(var, builder.create_int_constant(0));
  auto res = builder.create_isub(var, var_zero);
  auto ret_inst = builder.create_ret(res);
  dump_graph(graph, "PeepSubTest0");
  comp.run_all_passes();
  dump_graph(graph, "PeepSubTest1");
  ASSERT_EQ(bb0->size(), 2);
  ASSERT_EQ(bb0->front().get_opcode(), INST_CONST);
  ASSERT_EQ(&bb0->back(), ret_inst);
}

#undef MKBB
#undef CONNECT

} // namespace Tests
} // namespace koda