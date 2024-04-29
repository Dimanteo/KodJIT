#pragma once

#include <Core/LoopInfo.hpp>
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
  using loop_id_t = LoopInfo::loop_id_t;

private:
  bbid_t m_id = INVALID_BB;

  InstructionList m_instructions;

  BBVector m_predecessors{};

  // Max 2 successors are possible, since ISA doesn't have select operator
  BBVector m_successors{};

  ProgramGraph *m_graph = nullptr;

  loop_id_t m_loop_id = LoopInfo::NIL_LOOP_ID;

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

  bool is_in_loop() const { return m_loop_id != INVALID_BB; }

  bool is_loop_header() const { return m_loop_id == m_id; }

  void set_loop_id(loop_id_t loop_id) { m_loop_id = loop_id; }

  loop_id_t get_loop_id() const { return m_loop_id; }

  // Basic block iterators
  iterator begin() noexcept { return m_instructions.begin(); }
  iterator end() noexcept { return m_instructions.end(); }
  const_iterator begin() const noexcept { return m_instructions.cbegin(); }
  const_iterator end() const noexcept { return m_instructions.cend(); }
  auto rbegin() noexcept { return m_instructions.rbegin(); }
  auto rend() noexcept { return m_instructions.rend(); }

  SuccIterator succ_begin() { return m_successors.begin(); }
  SuccIterator succ_end() { return m_successors.end(); }

  PredIterator pred_begin() { return m_predecessors.begin(); }
  PredIterator pred_end() { return m_predecessors.end(); }
};

} // namespace koda
