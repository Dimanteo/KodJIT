#pragma once

#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>

#include <memory>
#include <vector>

namespace koda {

class ProgramGraph final {
  std::vector<BasicBlock> m_bb_arena{};
  std::vector<std::unique_ptr<Instruction>> m_inst_arena{};

  std::vector<IOperand *> params{};

  BasicBlock *m_entry = nullptr;

public:
  ProgramGraph() = default;

  BasicBlock *createBasicBlock();

  template <class InstT, typename... Args> InstT *createInstruction(Args &&...args) {
    instid_t id = m_inst_arena.size();
    auto inst_ptr = new InstT(id, std::forward<Args>(args)...);
    m_inst_arena.emplace_back(inst_ptr);
    return inst_ptr;
  }

  void setEntry(BasicBlock *bb) { m_entry = bb; }

  BasicBlock *getEntry() const { return m_entry; }
};

}; // namespace koda