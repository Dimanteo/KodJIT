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
  const std::vector<int> ref{0, 2, 4, 5, 11, 12, 13, 1, 3, 6, 7, 8, 9, 10, 14, 15};
  ASSERT_EQ(std::distance(linear_order.begin(), linear_order.end()), ref.size());
  unsigned i = 0;
  for (auto id = linear_order.begin(); id != linear_order.end(); ++id) {
    std::cout << (*id)->get_id() << " ";
    ASSERT_EQ((*id)->get_id(), ref[i++]);
  }
  dump_loops(comp, "LinearOrderTestLoops");
}

#undef MKBB
#undef CONNECT

} // namespace Tests
} // namespace koda