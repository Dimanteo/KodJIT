#include <IR/IRBuilder.hpp>
#include <IR/IRPrinter.hpp>
#include <IR/ProgramGraph.hpp>

#include <gtest/gtest.h>

#include <fstream>

namespace koda {

namespace Tests {

void dump_graph_and_loop_tree(ProgramGraph &graph, std::string filename) {
  std::ofstream dot_log(filename + ".dot", std::ios_base::out);
  IRPrinter printer(dot_log);
  printer.print_prog_graph(graph);
  dot_log.close();

  dot_log.open(filename + "_LoopTree.dot", std::ios_base::out);
  dot_log << "digraph {\n";
  for (auto &&loop_it : graph.get_loop_tree()) {
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
  dot_log << GraphPrinter::make_dot_graph(graph.get_loop_tree(), graph.get_loop_tree().get_root());
  dot_log << "}";
  dot_log.close();
}

void connect(BasicBlock *from, BasicBlock *to, IRBuilder &builder) {
  builder.set_insert_point(from);
  builder.create_branch(to);
}

void connect(BasicBlock *from, BasicBlock *left, BasicBlock *right, IRBuilder &builder) {
  builder.set_insert_point(from);
  auto dummy = builder.create_int_constant(10);
  builder.create_conditional_branch(CmpFlag::CMP_EQ, left, right, dummy, dummy);
}

void verify_loop(LoopInfo &loop, const std::vector<BasicBlock *> &ref) {
  for (auto &&bb : ref) {
    ASSERT_TRUE(bb->is_in_loop());
    ASSERT_EQ(bb->get_owner_loop_header(), loop.get_header());
  }

  std::vector<bbid_t> ref_ids, actual_ids;
  for (auto &&bb : loop) {
    actual_ids.push_back(bb->get_id());
  }
  for (auto &&bb : ref) {
    ref_ids.push_back(bb->get_id());
  }
  std::sort(actual_ids.begin(), actual_ids.end());
  std::sort(ref_ids.begin(), ref_ids.end());
  ASSERT_TRUE(std::equal(actual_ids.begin(), actual_ids.end(), ref_ids.begin(), ref_ids.end()));
}

TEST(LoopAnalyzerTests, loop_analyzer_ex1) {
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

  connect(A, B, builder);
  connect(B, D, C, builder);
  connect(D, E, builder);
  connect(E, B, builder);

  graph.build_dom_tree();

  auto &&tree = graph.get_dom_tree();

  ASSERT_EQ(tree.get_parent(B), A);
  ASSERT_EQ(tree.get_parent(C), B);
  ASSERT_EQ(tree.get_parent(D), B);
  ASSERT_EQ(tree.get_parent(E), D);

  graph.build_loop_tree();
  auto &loop_tree = graph.get_loop_tree();

  ASSERT_EQ(loop_tree.size(), 2);
  ASSERT_FALSE(A->is_in_loop());
  ASSERT_FALSE(C->is_in_loop());
  verify_loop(loop_tree.get(B->get_id()), {B, D, E});

  dump_graph_and_loop_tree(graph, "loop_ex1");
}

TEST(LoopAnalyzerTests, loop_ex2) {
  /*
              ┌─────────────────────────────┐
              ▼                             │
  ┌───┐     ┌───┐     ┌───┐     ┌───┐     ┌───┐
  │ A │ ──▶ │ B │ ──▶ │ C │ ──▶ │ D │ ──▶ │ E │
  └───┘     └───┘     └───┘     └───┘     └───┘
                        │         │
                        │         │
                        ▼         │
                      ┌───┐       │
                      │ F │ ◀─────┘
                      └───┘
  */
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *A = graph.create_basic_block();
  BasicBlock *B = graph.create_basic_block();
  BasicBlock *C = graph.create_basic_block();
  BasicBlock *D = graph.create_basic_block();
  BasicBlock *E = graph.create_basic_block();
  BasicBlock *F = graph.create_basic_block();

  builder.set_entry_point(A);

  connect(A, B, builder);
  connect(B, C, builder);
  connect(C, D, F, builder);
  connect(D, E, F, builder);
  connect(E, B, builder);

  graph.build_loop_tree();

  ASSERT_FALSE(A->is_in_loop());
  ASSERT_FALSE(F->is_in_loop());

  auto &loop_tree = graph.get_loop_tree();
  ASSERT_TRUE(loop_tree.contains(B->get_id()));
  ASSERT_EQ(loop_tree.size(), 2);

  verify_loop(loop_tree.get(B->get_id()), {B, C, D, E});

  dump_graph_and_loop_tree(graph, "loop_ex2");
}

TEST(LoopAnalyzerTests, loop_ex3) {
  /*
                 ┌───┐
              ┌▶ │ A │
              │  └───┘
              │    │
              │    │
              │    ▼
  ┌───┐       │  ┌───┐
  │ D │ ◀─────┼─ │ B │ ◀┐
  └───┘       │  └───┘  │
    │         │    │    │
    │         │    │    │
    │         │    ▼    │
    │         │  ┌───┐  │       ┌───┐
    │         │  │ C │ ─┼─────▶ │ E │
    │         │  └───┘  │       └───┘
    │         │    │    │
    │         │    │    │
    │         │    ▼    │
    │         │  ┌───┐  │
    └─────────┼▶ │ F │  │
              │  └───┘  │
              │    │    │
              │    │    │
              │    ▼    │
              │  ┌───┐  │
              │  │ G │ ─┘
              │  └───┘
              │    │
              │    │
              │    ▼
              │  ┌───┐
              └─ │ H │
                 └───┘
  */
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *A = graph.create_basic_block();
  BasicBlock *B = graph.create_basic_block();
  BasicBlock *C = graph.create_basic_block();
  BasicBlock *D = graph.create_basic_block();
  BasicBlock *E = graph.create_basic_block();
  BasicBlock *F = graph.create_basic_block();
  BasicBlock *G = graph.create_basic_block();
  BasicBlock *H = graph.create_basic_block();

  builder.set_entry_point(A);

  connect(A, B, builder);
  connect(B, C, D, builder);
  connect(C, E, F, builder);
  connect(D, F, builder);
  // E
  connect(F, G, builder);
  connect(G, H, B, builder);
  connect(H, A, builder);

  graph.build_loop_tree();

  ASSERT_FALSE(E->is_in_loop());

  auto &loop_tree = graph.get_loop_tree();
  ASSERT_EQ(loop_tree.size(), 3);

  ASSERT_TRUE(loop_tree.contains(A->get_id()));
  ASSERT_TRUE(loop_tree.contains(B->get_id()));

  verify_loop(loop_tree.get(A->get_id()), {A, H});
  verify_loop(loop_tree.get(B->get_id()), {B, C, D, F, G});

  dump_graph_and_loop_tree(graph, "loop_ex3");
}

TEST(LoopAnalyzerTests, loop_ex4) {
  /*
            ┌───┐
            │ 1 │
            └───┘
              │
              │
              ▼
  ┌───┐     ┌───┐
  │ 3 │ ◀── │ 2 │
  └───┘     └───┘
    │         │
    │         │
    │         ▼
    │       ┌───┐     ┌───┐
    │       │ 6 │ ──▶ │ 7 │
    │       └───┘     └───┘
    │         │         │
    │         │         │
    │         ▼         │
    │       ┌───┐       │
    │       │ 5 │       │
    │       └───┘       │
    │         │         │
    │         │         │
    │         ▼         │
    │       ┌───┐       │
    └─────▶ │ 4 │ ◀─────┘
            └───┘
  */
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb1 = graph.create_basic_block();
  BasicBlock *bb2 = graph.create_basic_block();
  BasicBlock *bb3 = graph.create_basic_block();
  BasicBlock *bb4 = graph.create_basic_block();
  BasicBlock *bb5 = graph.create_basic_block();
  BasicBlock *bb6 = graph.create_basic_block();
  BasicBlock *bb7 = graph.create_basic_block();

  builder.set_entry_point(bb1);

  connect(bb1, bb2, builder);
  connect(bb2, bb3, bb6, builder);
  connect(bb3, bb4, builder);
  connect(bb5, bb4, builder);
  connect(bb6, bb5, bb7, builder);
  connect(bb7, bb4, builder);

  graph.build_loop_tree();

  ASSERT_EQ(graph.get_loop_tree().size(), 1);
  for (auto &&bb : graph) {
    ASSERT_FALSE(bb->is_in_loop());
  }
}

TEST(LoopAnalyzerTests, loop_ex5) {
  /*
          ┌────┐
          │ 1  │
          └────┘
            │
            │
            ▼
          ┌────┐
  ┌─────▶ │ 2  │ ─┐
  │       └────┘  │
  │         │     │
  │         │     │
  │         ▼     │
  │       ┌────┐  │
  │       │ 4  │  │
  │       └────┘  │
  │         │     │
  │         │     │
  │         ▼     │
  │       ┌────┐  │
  │    ┌▶ │ 3  │ ◀┘
  │    │  └────┘
  │    │    │
  │    │    │
  │    │    ▼
  │    │  ┌────┐
  │    └─ │ 5  │
  │       └────┘
  │         │
  │         │
  │         ▼
  │       ┌────┐
  │       │ 6  │ ◀┐
  │       └────┘  │
  │         │     │
  │         │     │
  │         ▼     │
  │       ┌────┐  │
  │       │ 7  │ ─┘
  │       └────┘
  │         │
  │         │
  │         ▼
┌───┐     ┌────┐
│ 9 │ ◀── │ 8  │
└───┘     └────┘
            │
            │
            ▼
          ┌────┐
          │ 10 │
          └────┘
            │
            │
            ▼
          ┌────┐
          │ 11 │
          └────┘
  */
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb1 = graph.create_basic_block();
  BasicBlock *bb2 = graph.create_basic_block();
  BasicBlock *bb3 = graph.create_basic_block();
  BasicBlock *bb4 = graph.create_basic_block();
  BasicBlock *bb5 = graph.create_basic_block();
  BasicBlock *bb6 = graph.create_basic_block();
  BasicBlock *bb7 = graph.create_basic_block();
  BasicBlock *bb8 = graph.create_basic_block();
  BasicBlock *bb9 = graph.create_basic_block();
  BasicBlock *bb10 = graph.create_basic_block();
  BasicBlock *bb11 = graph.create_basic_block();

  builder.set_entry_point(bb1);

  connect(bb1, bb2, builder);
  connect(bb2, bb3, bb4, builder);
  connect(bb3, bb5, builder);
  connect(bb4, bb3, builder);
  connect(bb5, bb3, bb6, builder);
  connect(bb6, bb7, builder);
  connect(bb6, bb7, builder);
  connect(bb7, bb6, bb8, builder);
  connect(bb8, bb9, bb10, builder);
  connect(bb9, bb2, builder);
  connect(bb10, bb11, builder);

  graph.build_loop_tree();

  ASSERT_EQ(graph.get_loop_tree().size(), 4);

  ASSERT_FALSE(bb1->is_in_loop());
  ASSERT_FALSE(bb10->is_in_loop());
  ASSERT_FALSE(bb11->is_in_loop());

  auto &loop_tree = graph.get_loop_tree();
  ASSERT_TRUE(loop_tree.contains(bb2->get_id()));
  verify_loop(loop_tree.get(bb2->get_id()), {bb2, bb4, bb8, bb9});
  ASSERT_TRUE(loop_tree.contains(bb3->get_id()));
  verify_loop(loop_tree.get(bb3->get_id()), {bb3, bb5});
  ASSERT_TRUE(loop_tree.contains(bb6->get_id()));
  verify_loop(loop_tree.get(bb6->get_id()), {bb6, bb7});

  dump_graph_and_loop_tree(graph, "loop_ex5");
}

TEST(LoopAnalyzerTests, loop_ex6) {
  /*

       ┌────────────────────────┐
       │                        │
       │                 ┌───┐  │
       │                 │ 1 │  │
       │                 └───┘  │
       │                   │    │
       │                   │    │
       │                   ▼    │
     ┌───┐     ┌───┐     ┌───┐  │
     │ 6 │ ◀── │ 5 │ ◀── │ 2 │ ◀┘
     └───┘     └───┘     └───┘
       │         │         │
       │         │         │
       ▼         │         ▼
     ┌───┐       │       ┌───┐
  ┌─ │ 8 │       │       │ 3 │ ◀┐
  │  └───┘       │       └───┘  │
  │    │         │         │    │
  │    │         │         │    │
  │    │         │         ▼    │
  │    │         │       ┌───┐  │
  │    │         └─────▶ │ 4 │  │
  │    │                 └───┘  │
  │    │                   │    │
  │    │                   │    │
  │    │                   ▼    │
  │    │                 ┌───┐  │
  │    └───────────────▶ │ 7 │ ─┘
  │                      └───┘
  │                        │
  │                        │
  │                        ▼
  │                      ┌───┐
  │                      │ 9 │
  │                      └───┘
  │                        ▲
  └────────────────────────┘
  */
  ProgramGraph graph;
  IRBuilder builder(graph);

  BasicBlock *bb1 = graph.create_basic_block();
  BasicBlock *bb2 = graph.create_basic_block();
  BasicBlock *bb3 = graph.create_basic_block();
  BasicBlock *bb4 = graph.create_basic_block();
  BasicBlock *bb5 = graph.create_basic_block();
  BasicBlock *bb6 = graph.create_basic_block();
  BasicBlock *bb7 = graph.create_basic_block();
  BasicBlock *bb8 = graph.create_basic_block();
  BasicBlock *bb9 = graph.create_basic_block();

  builder.set_entry_point(bb1);

  connect(bb1, bb2, builder);
  connect(bb2, bb3, bb5, builder);
  connect(bb3, bb4, builder);
  connect(bb4, bb7, builder);
  connect(bb5, bb4, bb6, builder);
  connect(bb6, bb2, bb8, builder);
  connect(bb7, bb3, bb9, builder);
  connect(bb8, bb7, bb9, builder);

  graph.build_loop_tree();

  ASSERT_EQ(graph.get_loop_tree().size(), 3);

  ASSERT_FALSE(bb1->is_in_loop());
  ASSERT_FALSE(bb8->is_in_loop());
  ASSERT_FALSE(bb9->is_in_loop());

  auto &loop_tree = graph.get_loop_tree();
  ASSERT_TRUE(loop_tree.contains(bb2->get_id()));
  verify_loop(loop_tree.get(bb2->get_id()), {bb2, bb5, bb6});

  BasicBlock *irred_header = nullptr;
  for (auto &&bb : {bb3, bb4, bb7}) {
    if (loop_tree.contains(bb->get_id())) {
      irred_header = bb;
    }
  }
  ASSERT_NE(irred_header, nullptr);

  auto &irr_loop = loop_tree.get(irred_header->get_id());
  ASSERT_FALSE(irr_loop.is_reducible());

  dump_graph_and_loop_tree(graph, "loop_ex6");
}

} // namespace Tests

} // namespace koda