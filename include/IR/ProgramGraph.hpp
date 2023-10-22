#pragma once

#include "IR/IROperand.hpp"
#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>

#include <memory>
#include <vector>

namespace koda {

class ProgramGraph final {
  std::vector<BasicBlock> m_bb_arena{};
  std::vector<std::unique_ptr<Instruction>> m_inst_arena{};

  std::vector<ProgParam> m_params{};

  std::vector<IntConstOperand> m_consts{};

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

  void appendParam(OperandType type) { m_params.emplace_back(type); }

  void popParam() { m_params.pop_back(); }

  ProgParam *getParam(size_t idx) { return &m_params[idx]; }

  size_t getNumParams() const { return m_params.size(); }

  ProgParam *paramBack() { return &m_params.back(); }

  ProgParam *paramFront() { return &m_params.front(); }

  IntConstOperand *createIntConstant(uint64_t value) {
    m_consts.emplace_back(value);
    return &m_consts.back();
  }
};

}; // namespace koda