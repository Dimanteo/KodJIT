#include "Core/Analyses.hpp"

#include "Core/Compiler.h"
#include "DataStructures/Graph.hpp"
#include "IR/ProgramGraph.hpp"

namespace koda {

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
    m_blocks.push_back(header);
  }
  m_latches.push_back(latch);
  m_blocks.push_back(latch);
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
    header->set_owner_loop_header(header);
    latch->set_owner_loop_header(header);
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
    marked.clear();
    marked.resize(graph.size(), false);
    marked[header->get_id()] = true;
    auto &&loop = m_loop_tree.get(header->get_id());
    if (!loop.is_reducible()) {
      continue;
    }
    for (auto &&latch : loop.get_latches()) {
      visit_dfs_conditional</*Backward=*/true>(
          graph, latch,
          [this, &marked, &loop, header](BasicBlock *backedge_src) {
            if (marked[backedge_src->get_id()]) {
              return false;
            }
            marked[backedge_src->get_id()] = true;
            if (backedge_src->is_in_loop() &&
                backedge_src->get_owner_loop_header() != header) {
              m_loop_tree.link(header->get_id(),
                               backedge_src->get_owner_loop_header()->get_id());
            } else if (!backedge_src->is_in_loop()) {
              backedge_src->set_owner_loop_header(header);
              loop.add_block(backedge_src);
            }
            return true;
          });
    }
  }

  // Build tree
  m_loop_tree.insert(ROOT_LOOP_ID);
  m_loop_tree.set_root(ROOT_LOOP_ID);
  auto &root_loop = m_loop_tree.get(ROOT_LOOP_ID);
  root_loop.set_reducible(false);
  for (auto &&loop : m_loop_tree) {
    auto loop_id = loop.first;
    if (loop_id != ROOT_LOOP_ID && !m_loop_tree.has_parent(loop_id)) {
      m_loop_tree.link(ROOT_LOOP_ID, loop_id);
    }
  }
  for (auto &&bb : graph) {
    if (!bb->is_in_loop()) {
      root_loop.add_block(bb.get());
    }
  }
}

} // namespace koda