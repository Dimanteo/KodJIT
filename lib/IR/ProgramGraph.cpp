#include <IR/ProgramGraph.hpp>

namespace koda {

void LoopInfo::add_back_edge(BasicBlock *latch, BasicBlock *header) {
  assert(latch && "Invalid block");
  assert(header && "Invalid block");
  assert((m_header == nullptr || header == m_header) && "Back edge must lead to loop header");
  if (m_header == nullptr) {
    m_header = header;
    m_blocks.push_back(header);
  }
  m_latches.push_back(latch);
  m_blocks.push_back(latch);
}

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
  if (m_dom_tree.empty()) {
    build_loop_tree();
  }

  // Collect backedges
  std::vector<bool> marked(m_bb_arena.size(), false);
  std::vector<std::pair<BasicBlock *, BasicBlock *>> backedges;
  auto backedge_collect = [&marked, &backedges](BasicBlock *bb) {
    marked[bb->get_id()] = true;
    std::for_each(bb->succ_begin(), bb->succ_end(), [bb, &marked, &backedges](BasicBlock *succ) {
      if (marked[succ->get_id()]) {
        backedges.emplace_back(bb, succ);
      }
    });
  };
  auto unmark = [&marked](BasicBlock *bb) { marked[bb->get_id()] = false; };
  visit_dfs(*this, m_entry, backedge_collect, unmark);

  // Create loops
  for (auto &&edge : backedges) {
    auto header = edge.second;
    auto latch = edge.first;
    if (!m_loop_tree.contains(header->get_id())) {
      m_loop_tree.insert(header->get_id());
    }
    auto &&loop = m_loop_tree.get(header->get_id());
    loop.add_back_edge(latch, header);
    loop.set_reducible(loop.is_reducible() && m_dom_tree.is_dominator_of(header, latch));
    header->set_owner_loop_header(header);
    latch->set_owner_loop_header(header);
  }

  // Get headers in post order.
  std::vector<BasicBlock *> post_order;
  auto inserter = std::back_inserter(post_order);
  visit_dfs(*this, m_entry, [&inserter, this](BasicBlock *bb) {
    if (m_loop_tree.contains(bb->get_id())) {
      *inserter = bb;
    }
  });

  // Populate loops
  for (auto &&header : post_order) {
    marked.clear();
    marked.resize(m_bb_arena.size(), false);
    marked[header->get_id()] = true;
    auto &&loop = m_loop_tree.get(header->get_id());

    auto loop_adder = [this, header, &marked, &loop](BasicBlock *bb) {
      if (marked[bb->get_id()]) {
        return false;
      }
      marked[bb->get_id()] = true;
      if (bb->is_in_loop()) {
        m_loop_tree.link(header->get_id(), bb->get_owner_loop_header()->get_id());
      } else {
        bb->set_owner_loop_header(header);
        loop.add_block(bb);
      }
      return true;
    };

    if (!loop.is_reducible()) {
      continue;
    }
    for (auto &&latch : loop.get_latches()) {
      visit_dfs_conditional</*Backward=*/ true>(*this, latch, [this, &loop_adder, header](BasicBlock *backedge_src) {
        if (backedge_src == header) {
          return false;
        }
        visit_dfs_conditional(*this, backedge_src, loop_adder);
        return true;
      });
    }
  }

  // Build tree
  m_loop_tree.insert(ROOT_LOOP_ID);
  m_loop_tree.set_root(ROOT_LOOP_ID);
  auto root_loop = m_loop_tree.get(ROOT_LOOP_ID);
  for (auto &&loop : m_loop_tree) {
    auto loop_id = loop.first;
    if (loop_id != ROOT_LOOP_ID && !m_loop_tree.has_parent(loop_id)) {
      m_loop_tree.link(ROOT_LOOP_ID, loop_id);
    }
  }
  for (auto &&bb : *this) {
    if (!bb->is_in_loop()) {
      root_loop.add_block(bb.get());
    }
  }
}

} // namespace koda
