#pragma once

#include "IR/IROperand.hpp"
#include <DataStructures/DominatorTree.hpp>
#include <DataStructures/Graph.hpp>
#include <DataStructures/Tree.hpp>
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

  size_t get_index() const { return m_index; }
  OperandType get_type() const { return m_type; }
};

class LoopInfo {
  bool m_is_reducible = true;

  BasicBlock *m_header = nullptr;

  std::vector<BasicBlock *> m_blocks{};

  std::vector<BasicBlock *> m_latches{};

public:
  using iterator = std::vector<BasicBlock *>::iterator;

  BasicBlock *get_header() const { return m_header; }

  bool is_reducible() const { return m_is_reducible; }

  void add_back_edge(BasicBlock *latch, BasicBlock *header);

  void set_reducible(bool is_reducible) { m_is_reducible = is_reducible; }

  std::vector<BasicBlock *> &get_latches() { return m_latches; }

  void add_block(BasicBlock *bb) {
    m_blocks.push_back(bb);
  }

  iterator begin() { return m_blocks.begin(); }

  iterator end() { return m_blocks.end(); }
};

class ProgramGraph final {
public:
  using BasicBlockArena = std::vector<std::unique_ptr<BasicBlock>>;
  using iterator = BasicBlockArena::iterator;
  using BBPtr = std::unique_ptr<BasicBlock>;
  using loop_id_t = bbid_t;

  // Graph traits
  using NodeId = BasicBlock *;
  using PredIterator = BasicBlock::PredIterator;
  using SuccIterator = BasicBlock::SuccIterator;

  using DomsTree = DominatorTree<BasicBlock *>;
  using LoopTree = Tree<bbid_t, LoopInfo>;

private:

  BasicBlockArena m_bb_arena{};

  std::vector<std::unique_ptr<Instruction>> m_inst_arena{};

  std::vector<Parameter> m_params{};

  BasicBlock *m_entry = nullptr;

  DomsTree m_dom_tree{nullptr};

  constexpr static loop_id_t ROOT_LOOP_ID = -1;
  constexpr static loop_id_t NONE_LOOP_ID = -2;
  LoopTree m_loop_tree{NONE_LOOP_ID};

public:
  ProgramGraph() = default;
  ProgramGraph(const ProgramGraph &) = delete;
  ProgramGraph &operator=(const ProgramGraph &) = delete;

  BasicBlock *create_basic_block();

  template <class InstT, typename... Args> InstT *create_instruction(Args &&...args) {
    instid_t id = m_inst_arena.size();
    auto inst_ptr = new InstT(id, std::forward<Args>(args)...);
    m_inst_arena.emplace_back(inst_ptr);
    return inst_ptr;
  }

  BasicBlock *get_bb(bbid_t id) { return m_bb_arena[id].get(); }

  void set_entry(BasicBlock *bb) { m_entry = bb; }

  BasicBlock *get_entry() const { return m_entry; }

  // Create program parameter of given type.
  // Return index of that parameter
  //
  size_t create_param(OperandType type) {
    size_t idx = m_params.size();
    m_params.emplace_back(idx, type);
    return idx;
  }

  void pop_back_param() { m_params.pop_back(); }

  Parameter get_param(size_t idx) { return m_params[idx]; }

  size_t get_num_params() const { return m_params.size(); }

  iterator begin() { return m_bb_arena.begin(); }
  iterator end() { return m_bb_arena.end(); }

  void build_dom_tree();

  const DomsTree &get_dom_tree() const { return m_dom_tree; }

  void build_loop_tree();

  LoopTree &get_loop_tree() { return m_loop_tree; }

  // Graph traits

  static PredIterator pred_begin([[maybe_unused]] ProgramGraph &owner, NodeId node) {
    return node->pred_begin();
  }
  static PredIterator pred_end(ProgramGraph &owner, NodeId node) {
    (void)owner;
    return node->pred_end();
  }

  static SuccIterator succ_begin(ProgramGraph &owner, NodeId node) {
    (void)owner;
    return node->succ_begin();
  }
  static SuccIterator succ_end(ProgramGraph &owner, NodeId node) {
    (void)owner;
    return node->succ_end();
  }

  // Printable graph traits
  static std::string node_to_string(ProgramGraph &graph, NodeId node) {
    (void)graph;
    return std::to_string(reinterpret_cast<uint64_t>(node));
  }
};

}; // namespace koda