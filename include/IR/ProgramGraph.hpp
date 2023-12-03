#pragma once

#include "IR/IROperand.hpp"
#include <DataStructures/Graph.hpp>
#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>

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
  using iterator = BasicBlockArena::iterator;
  using BBPtr = std::unique_ptr<BasicBlock>;

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

  BasicBlock *getBB(bbid_t id) { return m_bb_arena[id].get(); }

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

  iterator begin() { return m_bb_arena.begin(); }
  iterator end() { return m_bb_arena.end(); }

  // Printable graph traits
  using NodeId = BasicBlock *;
  static std::string nodeToString(ProgramGraph &graph, NodeId node) {
    (void)graph;
    return std::to_string(reinterpret_cast<uint64_t>(node));
  }
};

template <> struct GraphTraits<ProgramGraph> {
  using NodeId = BasicBlock *;
  using PredIterator = BasicBlock::PredIterator;
  using SuccIterator = BasicBlock::SuccIterator;

  static PredIterator predBegin([[maybe_unused]] ProgramGraph &owner, NodeId node) {
    return node->predBegin();
  }
  static PredIterator predEnd(ProgramGraph &owner, NodeId node) {
    (void)owner;
    return node->predEnd();
  }

  static SuccIterator succBegin(ProgramGraph &owner, NodeId node) {
    (void)owner;
    return node->succBegin();
  }
  static SuccIterator succEnd(ProgramGraph &owner, NodeId node) {
    (void)owner;
    return node->succEnd();
  }
};

}; // namespace koda