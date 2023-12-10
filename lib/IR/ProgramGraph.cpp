#include <IR/ProgramGraph.hpp>

namespace koda {
BasicBlock *ProgramGraph::create_basic_block() {
  bbid_t id = m_bb_arena.size();
  auto new_bb = new BasicBlock(id, *this);
  m_bb_arena.emplace_back(new_bb);
  return m_bb_arena.back().get();
}

void ProgramGraph::build_dom_tree() {
  if (!m_dom_tree.empty()) {
    m_dom_tree.clear();
  }
  DominatorTreeBuilder<ProgramGraph> builder;
  builder.build_tree(*this, m_entry, m_dom_tree);
}

void ProgramGraph::build_loop_tree() {
  // std::vector<bool> in_path(m_bb_arena.size());
  // auto backedge_collect = [&in_path](BasicBlock *bb) {
  //   in_path[bb->get_id()] = true;

  // };
  // visit_dfs(*this, m_entry, back_edge_collect);
}

} // namespace koda
