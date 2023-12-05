#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>
#include <IR/Instruction.hpp>

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

  BasicBlock *get_successor(SuccessorIdx idx) const {
    if (idx >= m_successors.size())
      return nullptr;
    return m_successors[idx];
  }

  void set_successor(SuccessorIdx idx, BasicBlock *succ) {
    if (idx >= m_successors.size()) {
      m_successors.resize(idx + 1);
    }
    m_successors[idx] = succ;
    succ->add_predecessor(this);
  }

  void set_false_successor(BasicBlock *bb) { m_successors[FALSE_IDX] = bb; }

  void set_true_successor(BasicBlock *bb) { m_successors[TRUE_IDX] = bb; }

public:
  BasicBlock(bbid_t id, ProgramGraph &graph) : m_id(id), m_graph(&graph) {}

  bbid_t get_id() const { return m_id; }

  void add_instruction(Instruction *instruction) {
    m_instructions.insert_tail(instruction);
    instruction->set_bb(this);
  }

  Instruction *remove_instruction(Instruction *instruction) {
    instruction->set_bb(nullptr);
    return m_instructions.remove(instruction);
  }

  // Returns unconditional successor
  BasicBlock *get_uncond_successor() const { return get_successor(UNCOND_IDX); }

  BasicBlock *get_false_successor() const { return get_successor(FALSE_IDX); }

  BasicBlock *get_true_successor() const { return get_successor(TRUE_IDX); }

  void set_uncond_successor(BasicBlock *bb) { set_successor(UNCOND_IDX, bb); }

  void set_cond_successors(BasicBlock *false_bb, BasicBlock *true_bb);

  bool has_successor() const;

  void add_predecessor(BasicBlock *pred) { m_predecessors.push_back(pred); }

  OperandType get_type() const override { return OperandType::LABEL; }

  // Basic block iterators
  iterator begin() noexcept { return m_instructions.begin(); }
  iterator end() noexcept { return m_instructions.end(); }

  SuccIterator succ_begin() { return m_successors.begin(); }
  SuccIterator succ_end() { return m_successors.end(); }

  PredIterator pred_begin() { return m_predecessors.begin(); }
  PredIterator pred_end() { return m_predecessors.end(); }
};

} // namespace koda
