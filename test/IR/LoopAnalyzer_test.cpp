#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>
#include <IR/IRPrinter.hpp>

#include <gtest/gtest.h>

#include <fstream>

namespace koda {

namespace Tests {

void dumpCFG(const char *test_name, ProgramGraph &prog) {
  std::ofstream dot_log(test_name, std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.print_prog_graph(prog);
  dot_log.close();
}

void dump_graph_and_dom_tree(ProgramGraph &graph, std::string filename) {
  std::ofstream dot_log(filename + ".dot", std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.print_prog_graph(graph);
  dot_log.close();

  dot_log.open(filename + "_LoopTree.dot", std::ios_base::out);
  GraphPrinter::print_dot(graph.get_loop_tree(), graph.get_loop_tree().get_root(), dot_log);
  dot_log.close();
}

TEST(LoopAnalyzeTests, loop_analyzer_ex1) {
  /*
          ┌───┐
          │ A │
          └───┘
            │
            │
            ▼
┌───┐     ┌───┐
│ C │ ◀── │ B │ ◀┐
└───┘     └───┘  │
            │    │
            │    │
            ▼    │
          ┌───┐  │
          │ D │  │
          └───┘  │
            │    │
            │    │
            ▼    │
          ┌───┐  │
          │ E │ ─┘
          └───┘
  */
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *A = graph.create_basic_block();
  BasicBlock *B = graph.create_basic_block();
  BasicBlock *C = graph.create_basic_block();
  BasicBlock *D = graph.create_basic_block();
  BasicBlock *E = graph.create_basic_block();

  builder.set_entry_point(A);

  builder.set_insert_point(A);
  builder.create_branch(B);

  builder.set_insert_point(B);
  auto dummy = builder.create_int_constant(10);
  builder.create_conditional_branch(CmpFlag::CMP_EQ, D, C, dummy, dummy);

  builder.set_insert_point(D);
  builder.create_branch(E);

  builder.set_insert_point(E);
  builder.create_branch(B);

  dumpCFG("bb_dom_tree.dot", graph);

  graph.build_dom_tree();

  auto &&tree = graph.get_dom_tree();

  ASSERT_EQ(tree.get_parent(B), A);
  ASSERT_EQ(tree.get_parent(C), B);
  ASSERT_EQ(tree.get_parent(D), B);
  ASSERT_EQ(tree.get_parent(E), D);

  graph.build_loop_tree();
  dump_graph_and_dom_tree(graph, "loop_ex1");
}

} // namespace Tests

} // namespace koda