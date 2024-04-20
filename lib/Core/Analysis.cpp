#include "Core/Analysis.hpp"

#include "Core/Compiler.h"
#include "DataStructures/Graph.hpp"
#include "IR/ProgramGraph.hpp"

namespace koda {

LoopInfo::loop_id_t LoopInfo::get_id() const {
  return m_header ? m_header->get_loop_id() : INVALID_LOOP_ID;
}

void RPOAnalysis::run(ProgramGraph &graph) {
  assert(graph.get_entry() != nullptr && "Entry block must be specified");
  m_rpo.clear();
  auto inserter = std::back_inserter(m_rpo);
  visit_rpo(graph, graph.get_entry(),
            [&inserter](BasicBlock *bb) { *inserter = bb->get_id(); });
}

void DomsTreeAnalysis::run(ProgramGraph &graph) {
  assert(graph.get_entry() != nullptr && "Entry block must be specified");
  if (!m_dom_tree.empty()) {
    m_dom_tree.clear();
  }
  DominatorTreeBuilder<ProgramGraph> builder;
  builder.build_tree(graph, graph.get_entry(), m_dom_tree);
}

void LoopInfo::add_back_edge(BasicBlock *latch, BasicBlock *header) {
  assert(latch && "Invalid block");
  assert(header && "Invalid block");
  assert((m_header == nullptr || header == m_header) &&
         "Back edge must lead to loop header");
  if (m_header == nullptr) {
    m_header = header;
  }
  m_latches.push_back(latch);
}

void LoopTreeAnalysis::run(Compiler &comp) {
  auto &&doms = comp.get_or_create<DomsTreeAnalysis>(comp.graph());
  run(comp.graph(), doms.get());
}

void LoopTreeAnalysis::run(ProgramGraph &graph,
                           DomsTreeAnalysis::DomsTree &dom_tree) {
  assert(graph.get_entry() != nullptr && "Entry block must be specified");

  // Collect backedges
  std::vector<bool> marked(graph.size(), false);
  std::vector<std::pair<BasicBlock *, BasicBlock *>> backedges;
  auto backedge_collect = [&marked, &backedges](BasicBlock *bb) {
    marked[bb->get_id()] = true;
    std::for_each(bb->succ_begin(), bb->succ_end(),
                  [bb, &marked, &backedges](BasicBlock *succ) {
                    if (marked[succ->get_id()]) {
                      backedges.emplace_back(bb, succ);
                    }
                  });
  };
  auto unmark = [&marked](BasicBlock *bb) { marked[bb->get_id()] = false; };
  visit_dfs(graph, graph.get_entry(), backedge_collect, unmark);

  // Create loops
  for (auto &&edge : backedges) {
    auto header = edge.second;
    auto latch = edge.first;
    if (!m_loop_tree.contains(header->get_id())) {
      m_loop_tree.insert(header->get_id());
    }
    auto &&loop = m_loop_tree.get(header->get_id());
    loop.add_back_edge(latch, header);
    loop.set_reducible(loop.is_reducible() &&
                       dom_tree.is_dominator_of(header, latch));
    header->set_loop_id(header->get_id());
    latch->set_loop_id(header->get_id());
  }

  // Get headers in post order.
  std::vector<BasicBlock *> post_order;
  auto inserter = std::back_inserter(post_order);
  visit_dfs_postorder(graph, graph.get_entry(),
                      [&inserter, this](BasicBlock *bb) {
                        if (m_loop_tree.contains(bb->get_id())) {
                          *inserter = bb;
                        }
                      });

  // Populate loops
  for (auto &&header : post_order) {
    auto &&loop = m_loop_tree.get(header->get_id());
    if (!loop.is_reducible()) {
      continue;
    }
    std::unordered_set<BasicBlock *> loop_blocks;
    loop_blocks.insert(header);
    for (auto &&latch : loop.get_latches()) {
      visit_dfs_conditional</*Backward=*/true>(
          graph, latch,
          [this, header, &loop_blocks](BasicBlock *backedge_src) {
            if (loop_blocks.find(backedge_src) != loop_blocks.end()) {
              return false;
            }
            loop_blocks.insert(backedge_src);
            if (backedge_src->is_in_loop() &&
                backedge_src->get_loop_id() != header->get_id()) {
              m_loop_tree.link(header->get_id(), backedge_src->get_loop_id());
            } else if (!backedge_src->is_in_loop()) {
              backedge_src->set_loop_id(header->get_id());
            }
            return true;
          });
      // Put blocks inside loop in DFS order
      visit_dfs_conditional(graph, header, [&loop_blocks, &loop](BasicBlock *bb) {
        if (loop_blocks.find(bb) == loop_blocks.end()) {
          return false;
        }
        loop.add_block(bb);
        return true;
      });
    }
  }

  // Build tree
  m_loop_tree.insert(LoopInfo::NIL_LOOP_ID);
  m_loop_tree.set_root(LoopInfo::NIL_LOOP_ID);
  auto &root_loop = m_loop_tree.get(LoopInfo::NIL_LOOP_ID);
  root_loop.set_reducible(false);
  for (auto &&loop : m_loop_tree) {
    auto loop_id = loop.first;
    if (loop_id != LoopInfo::NIL_LOOP_ID && !m_loop_tree.has_parent(loop_id)) {
      m_loop_tree.link(LoopInfo::NIL_LOOP_ID, loop_id);
    }
  }
  for (auto &&bb : graph) {
    if (!bb.is_in_loop()) {
      root_loop.add_block(&bb);
    }
  }
}

const LoopInfo &LoopTreeAnalysis::get_loop(const BasicBlock &bb) const {
  return m_loop_tree.get(bb.get_loop_id());
}

void LinearOrder::linearize_graph(Compiler &comp) {
  ProgramGraph &graph = comp.graph();
  const auto &loops = comp.get_or_create<LoopTreeAnalysis>(comp);
  const auto &rpo = comp.get_or_create<RPOAnalysis>(graph);
  std::vector<bool> visited(graph.size(), false);
  for (auto &&bb_id : rpo) {
    if (visited[bb_id]) {
      continue;
    }
    BasicBlock *bb = graph.get_bb(bb_id);
    if (bb->is_loop_header() && loops.get_loop(*bb).is_reducible()) {
      linearize_loop(*bb, loops, visited);
    } else {
      m_linear_order.push_back(bb);
      visited[bb_id] = true;
    }
  }
}

void LinearOrder::linearize_loop(const BasicBlock &header,
                                 const LoopTreeAnalysis &loops,
                                 std::vector<bool> &visited) {
  auto &&loop = loops.get_loop(header);
  for (auto &&bb : loop) {
    if (visited[bb->get_id()]) {
      continue;
    }

    // Inner loops
    if (bb->is_loop_header() && bb->get_loop_id() != loop.get_id()) {
      linearize_loop(*bb, loops, visited);
      continue;
    }

    visited[bb->get_id()] = true;
    m_linear_order.push_back(bb);
  }
}

void LinearOrder::run(Compiler &comp) { linearize_graph(comp); }

} // namespace koda