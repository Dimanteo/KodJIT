#pragma once

#include <vector>
#include "IR/IRTypes.hpp"

namespace koda {

class BasicBlock;

class LoopInfo {
  bool m_is_reducible = true;

  BasicBlock *m_header = nullptr;

  std::vector<BasicBlock *> m_blocks{};

  std::vector<BasicBlock *> m_latches{};

public:
  using loop_id_t = bbid_t;
  constexpr static loop_id_t NIL_LOOP_ID = INVALID_BB;
  constexpr static loop_id_t INVALID_LOOP_ID = -2;

  BasicBlock *get_header() const { return m_header; }

  loop_id_t get_id() const;

  bool is_reducible() const { return m_is_reducible; }

  void add_back_edge(BasicBlock *latch, BasicBlock *header);

  void set_reducible(bool is_reducible) { m_is_reducible = is_reducible; }

  std::vector<BasicBlock *> &get_latches() { return m_latches; }

  void add_block(BasicBlock *bb) { m_blocks.push_back(bb); }

  auto begin() const { return m_blocks.begin(); }

  auto end() const { return m_blocks.end(); }

  auto begin() { return m_blocks.begin(); }

  auto end() { return m_blocks.end(); }

};


}