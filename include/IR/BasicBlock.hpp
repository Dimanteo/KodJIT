#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>
#include <IR/Instruction.hpp>

#include <array>
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

  enum SuccessorIdx { FALSE_IDX = 0, UNCOND_IDX = 0, TRUE_IDX = 1 };

  BasicBlock *getSuccessor(SuccessorIdx idx) const { return m_successors[idx]; }

public:
  BasicBlock(bbid_t id, ProgramGraph &graph) : m_id(id), m_graph(&graph) {}

  void addInstruction(Instruction *instruction);

  Instruction *removeInstruction(Instruction *instruction);

  // Returns unconditional successor
  BasicBlock *getUncondSuccessor() const { return getSuccessor(UNCOND_IDX); }

  BasicBlock *getFalseSuccessor() const { return getSuccessor(FALSE_IDX); }

  BasicBlock *getTrueSuccessor() const { return getSuccessor(TRUE_IDX); }

  void setUncondSuccessor(BasicBlock *bb) { m_successors[UNCOND_IDX] = bb; }

  void setFalseSuccessor(BasicBlock *bb) { m_successors[FALSE_IDX] = bb; }

  void setTrueSuccessor(BasicBlock *bb) { m_successors[TRUE_IDX] = bb; }

  bool hasTerminator() const { return m_instructions.getTail()->isTerminator(); }

  using succ_iterator = SuccessorsArray::iterator;

  succ_iterator succ_begin() { return m_successors.begin(); }

  succ_iterator succ_end() { return m_successors.end(); }

  OperandType getType() const override { return OperandType::LABEL; }

  bool isTrackingUsers() const override { return false; }
};

} // namespace koda
