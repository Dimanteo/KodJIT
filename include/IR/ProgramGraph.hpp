#pragma once

#include "IR/IROperand.hpp"
#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace koda {

class Parameter {
  static constexpr ssize_t INVALID_IDX = std::numeric_limits<size_t>::max();

  size_t m_index = INVALID_IDX;
  OperandType m_type = OperandType::TYPE_INVALID;

public:
  Parameter(int index, OperandType type) : m_index(index), m_type(type) {}

  size_t getIndex() const { return m_index; }
  OperandType getType() const { return m_type; }
};

class ProgramGraph final {
public:
  using BasicBlockArena = std::vector<std::unique_ptr<BasicBlock>>;

private:
  BasicBlockArena m_bb_arena{};

  std::vector<std::unique_ptr<Instruction>> m_inst_arena{};

  std::vector<Parameter> m_params{};

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

  // Create program parameter of given type.
  // Return index of that parameter
  //
  size_t createParam(OperandType type) {
    size_t idx = m_params.size();
    m_params.emplace_back(idx, type);
    return idx;
  }

  void popBackParam() { m_params.pop_back(); }

  Parameter getParam(size_t idx) { return m_params[idx]; }

  size_t getNumParams() const { return m_params.size(); }

  // TODO: iterators
  void for_each_block(std::function<void(BasicBlock &)> funct) const {
    for (const auto &bb_ptr : m_bb_arena) {
      funct(*bb_ptr.get());
    }
  }
};

}; // namespace koda