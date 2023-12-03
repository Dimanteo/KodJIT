#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>
#include <IR/Instruction.hpp>

#include <array>
#include <vector>

namespace koda {

class ProgramGraph;
class BasicBlock;

class BasicBlock final : public IOperand {
public:
  using BBVector = std::vector<BasicBlock *>;
  using InstructionList = IntrusiveList<Instruction>;
  using iterator = InstructionList::iterator;
  using const_iterator = InstructionList::const_iterator;
  using PredIterator = BBVector::iterator;
  using SuccIterator = BBVector::iterator;

private:
  bbid_t m_id;

  InstructionList m_instructions;

  BBVector m_predecessors{};

  // Max 2 successors are possible, since ISA doesn't have select operator
  BBVector m_successors{};

  ProgramGraph *m_graph = nullptr;

  enum SuccessorIdx : unsigned { FALSE_IDX = 0, UNCOND_IDX = 0, TRUE_IDX = 1 };

  BasicBlock *getSuccessor(SuccessorIdx idx) const {
    if (idx >= m_successors.size())
      return nullptr;
    return m_successors[idx];
  }

  void setSuccessor(SuccessorIdx idx, BasicBlock *succ) {
    if (idx >= m_successors.size()) {
      m_successors.resize(idx + 1);
    }
    m_successors[idx] = succ;
    succ->addPredecessor(this);
  }

  void setFalseSuccessor(BasicBlock *bb) { m_successors[FALSE_IDX] = bb; }

  void setTrueSuccessor(BasicBlock *bb) { m_successors[TRUE_IDX] = bb; }

public:
  BasicBlock(bbid_t id, ProgramGraph &graph) : m_id(id), m_graph(&graph) {}

  bbid_t getID() const { return m_id; }

  void addInstruction(Instruction *instruction) {
    m_instructions.insertTail(instruction);
    instruction->setBB(this);
  }

  Instruction *removeInstruction(Instruction *instruction) {
    instruction->setBB(nullptr);
    return m_instructions.remove(instruction);
  }

  // Returns unconditional successor
  BasicBlock *getUncondSuccessor() const { return getSuccessor(UNCOND_IDX); }

  BasicBlock *getFalseSuccessor() const { return getSuccessor(FALSE_IDX); }

  BasicBlock *getTrueSuccessor() const { return getSuccessor(TRUE_IDX); }

  void setUncondSuccessor(BasicBlock *bb) { setSuccessor(UNCOND_IDX, bb); }

  void setCondSuccessors(BasicBlock *false_bb, BasicBlock *true_bb);

  bool hasSuccessor() const;

  void addPredecessor(BasicBlock *pred) { m_predecessors.push_back(pred); }

  OperandType getType() const override { return OperandType::LABEL; }

  // Basic block iterators
  iterator begin() noexcept { return m_instructions.begin(); }
  iterator end() noexcept { return m_instructions.end(); }

  SuccIterator succBegin() { return m_successors.begin(); }
  SuccIterator succEnd() { return m_successors.end(); }

  PredIterator predBegin() { return m_predecessors.begin(); }
  PredIterator predEnd() { return m_predecessors.end(); }
};

} // namespace koda
