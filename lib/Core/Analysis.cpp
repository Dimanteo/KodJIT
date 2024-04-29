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
          graph, latch, [this, header, &loop_blocks](BasicBlock *backedge_src) {
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
      visit_dfs_conditional(graph, header,
                            [&loop_blocks, &loop](BasicBlock *bb) {
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

void Liveness::run(Compiler &compiler) {
  using LiveSet = std::unordered_set<instid_t>;
  using BBLiveSetMap = std::vector<LiveSet>;

  auto &&linear_order = compiler.get_or_create<LinearOrder>(compiler);
  auto &&loop_analysis = compiler.get_or_create<LoopTreeAnalysis>(compiler);
  const size_t bb_count = compiler.graph().size();
  const size_t inst_count = compiler.graph().get_instr_count();
  std::vector<size_t> live_numbers(inst_count);
  RangeMap bb_live_nums(bb_count);
  BBLiveSetMap live_set_map(bb_count);
  m_live_ranges.resize(inst_count, {0, 0});

  auto set_live_num = [&live_numbers](instid_t iid, size_t num) {
    live_numbers[iid] = num;
  };

  auto get_live_num = [&live_numbers](instid_t iid) {
    return live_numbers[iid];
  };

  auto get_live_set = [&live_set_map](BasicBlock *bb) -> LiveSet & {
    return live_set_map[bb->get_id()];
  };

  auto get_bb_live_range = [&bb_live_nums](BasicBlock *bb) -> LiveRange & {
    return bb_live_nums[bb->get_id()];
  };

  // Assign live numbers
  size_t live_num = 0;
  for (const auto &bb : linear_order) {
    auto &&bb_range = get_bb_live_range(bb);
    bb_range.first = live_num;
    for (auto &&inst : *bb) {
      if (inst.is_phi()) {
        set_live_num(inst.get_id(), bb_range.first);
      } else {
        live_num += 2;
        set_live_num(inst.get_id(), live_num);
      }
    }
    live_num += 2;
    bb_range.second = live_num;
  }
  // Calculate live ranges
  for (auto &&bb_it = linear_order.rbegin(), end_bb = linear_order.rend();
       bb_it != end_bb; ++bb_it) {
    auto &&bb = *bb_it;
    // Calculate initial live set for block
    auto &&live_set = get_live_set(bb);
    for (auto &&succ = bb->succ_begin(), end_succ = bb->succ_end();
         succ != end_succ; ++succ) {
      // Union of all successors live sets
      auto &&succ_live_set = get_live_set(*succ);
      std::copy(succ_live_set.begin(), succ_live_set.end(),
                std::inserter(live_set, live_set.begin()));
      // Successor's phi inputs
      for (auto &&inst : **succ) {
        if (!inst.is_phi()) {
          continue;
        }
        PhiInstruction &phi = dynamic_cast<PhiInstruction &>(inst);
        auto phi_input = phi.get_value_for(bb);
        if (phi_input) {
          live_set.insert(phi_input->get_id());
        }
      }
    }
    // Append block live range to all entries in live set
    auto &&bb_range = get_bb_live_range(bb);
    for (instid_t iid : live_set) {
      extend_liverange(iid, bb_range);
    }
    // Shorten live ranges
    for (auto &&inst_it = bb->rbegin(), end_it = bb->rend(); inst_it != end_it;
         ++inst_it) {
      auto &&inst = *inst_it;
      size_t inst_live_num = get_live_num(inst.get_id());
      if (inst.is_def()) {
        m_live_ranges[inst.get_id()].first = inst_live_num;
        live_set.erase(inst.get_id());
      }
      if (inst.is_phi()) {
        continue;
      }
      for (auto &&input = inst.inputs_begin(), end_input = inst.inputs_end();
           input != end_input; ++input) {
        instid_t input_id = (*input)->get_id();
        live_set.insert(input_id);
        extend_liverange(input_id, {bb_range.first, inst_live_num});
      }
    }
    // Extend liveness in loops
    if (bb->is_loop_header()) {
      auto &&loop = loop_analysis.get_loop(*bb);
      size_t loop_end = bb_range.second;
      for (auto &&loop_bb : loop) {
        loop_end = std::max(loop_end, get_bb_live_range(loop_bb).second);
      }
      for (instid_t iid : live_set) {
        extend_liverange(iid, {bb_range.first, loop_end});
      }
    }
  }
}

} // namespace koda