#include <IR/ProgramGraph.hpp>

namespace koda {
BasicBlock *ProgramGraph::createBasicBlock() {
    bbid_t id = m_bb_arena.size();
    m_bb_arena.emplace_back(id, *this);
    return &m_bb_arena.back();
}
} // namespace koda
