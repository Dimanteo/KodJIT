#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>
#include <IR/Instruction.hpp>

#include <array>
#include <functional>
#include <iostream>
#include <vector>

namespace koda {

class ProgramGraph;

class BasicBlock final : public IOperand {
public:
  using BBVector = std::vector<BasicBlock *>;
  using SuccessorsArray = std::array<BasicBlock *, 2>;
  using InstructionList = IntrusiveList<Instruction>;

private:
  bbid_t m_id;

  InstructionList m_instructions;

  BBVector m_predecessors;

  // Max 2 successors are possible, since ISA doesn't have select operator
  SuccessorsArray m_successors{nullptr, nullptr};

  ProgramGraph *m_graph = nullptr;

  enum SuccessorIdx : unsigned { FALSE_IDX = 0, UNCOND_IDX = 0, TRUE_IDX = 1 };

  BasicBlock *getSuccessor(SuccessorIdx idx) const { return m_successors[idx]; }

  void setFalseSuccessor(BasicBlock *bb) { m_successors[FALSE_IDX] = bb; }

  void setTrueSuccessor(BasicBlock *bb) { m_successors[TRUE_IDX] = bb; }

public:
  BasicBlock(bbid_t id, ProgramGraph &graph) : m_id(id), m_graph(&graph) {}

  bbid_t getID() const { return m_id; }

  void addInstruction(Instruction *instruction) {
    m_instructions.insertTail(instruction);
    instruction->setBB(this);
  }

  Instruction *removeInstruction(Instruction *instruction);

  // Returns unconditional successor
  BasicBlock *getUncondSuccessor() const { return getSuccessor(UNCOND_IDX); }

  BasicBlock *getFalseSuccessor() const { return getSuccessor(FALSE_IDX); }

  BasicBlock *getTrueSuccessor() const { return getSuccessor(TRUE_IDX); }

  void setUncondSuccessor(BasicBlock *bb) { m_successors[UNCOND_IDX] = bb; }

  void setCondSuccessors(BasicBlock *false_bb, BasicBlock *true_bb);

  bool hasSuccessor() const;

  void addPredecessor(BasicBlock *pred) { m_predecessors.push_back(pred); }

  OperandType getType() const override { return OperandType::LABEL; }

  void for_each_succ(std::function<void(BasicBlock &)> funct) const;

  // Basic block iterators
  using iterator = InstructionList::iterator;
  using const_iterator = InstructionList::const_iterator;

  iterator begin() noexcept { return m_instructions.begin(); }
  iterator end() noexcept { return m_instructions.end(); }

  // const_iterator cbegin() const noexcept { return m_instructions.cbegin(); }
  // const_iterator cend() const noexcept { return m_instructions.cend(); }
};

} // namespace koda
