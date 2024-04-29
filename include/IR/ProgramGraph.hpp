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

  size_t get_index() const { return m_index; }
  OperandType get_type() const { return m_type; }
};

class ProgramGraph final {
public:
  using BasicBlockArena = std::vector<std::unique_ptr<BasicBlock>>;
  using BBPtr = std::unique_ptr<BasicBlock>;

  // Graph traits
  using NodeId = BasicBlock *;
  using PredIterator = BasicBlock::PredIterator;
  using SuccIterator = BasicBlock::SuccIterator;

  class BasicBlockIterator {
    using BaseIterator = BasicBlockArena::iterator;
    using value_type = BasicBlock;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = BasicBlockArena::iterator::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;

    BaseIterator iter;

  public:
    BasicBlockIterator(BaseIterator iter) : iter(iter) {}
    reference operator*() const noexcept { return *iter->get(); }
    BasicBlockIterator &operator++() noexcept {
      iter++;
      return *this;
    }
    BasicBlockIterator &operator++(int) noexcept {
      ++iter;
      return *this;
    }
    BasicBlockIterator &operator--() noexcept {
      iter--;
      return *this;
    }
    BasicBlockIterator &operator--(int) noexcept {
      --iter;
      return *this;
    }
    pointer operator->() const noexcept { return iter->get(); }
    bool is_equal(const BasicBlockIterator &other) const noexcept {
      return this->iter == other.iter;
    }
  };

  using iterator = BasicBlockIterator;

private:
  BasicBlockArena m_bb_arena{};

  std::vector<std::unique_ptr<Instruction>> m_inst_arena{};

  std::vector<Parameter> m_params{};

  BasicBlock *m_entry = nullptr;

public:
  ProgramGraph() = default;
  ProgramGraph(const ProgramGraph &) = delete;
  ProgramGraph &operator=(const ProgramGraph &) = delete;

  BasicBlock *create_basic_block();

  template <class InstT, typename... Args>
  InstT *create_instruction(Args &&...args) {
    instid_t id = m_inst_arena.size();
    auto inst_ptr = new InstT(id, std::forward<Args>(args)...);
    m_inst_arena.emplace_back(inst_ptr);
    return inst_ptr;
  }

  BasicBlock *get_bb(bbid_t id) const { return m_bb_arena[id].get(); }

  void set_entry(BasicBlock *bb) { m_entry = bb; }

  BasicBlock *get_entry() const { return m_entry; }

  size_t size() const { return m_bb_arena.size(); }

  size_t get_instr_count() const { return m_inst_arena.size(); }

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

  iterator begin() { return BasicBlockIterator(m_bb_arena.begin()); }
  iterator end() { return BasicBlockIterator(m_bb_arena.end()); }

  // Graph traits

  static PredIterator pred_begin([[maybe_unused]] ProgramGraph &owner,
                                 NodeId node) {
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

inline bool operator==(const ProgramGraph::BasicBlockIterator &lhs,
                       const ProgramGraph::BasicBlockIterator &rhs) {
  return lhs.is_equal(rhs);
}

inline bool operator!=(const ProgramGraph::BasicBlockIterator &lhs,
                       const ProgramGraph::BasicBlockIterator &rhs) {
  return !(lhs == rhs);
}

}; // namespace koda