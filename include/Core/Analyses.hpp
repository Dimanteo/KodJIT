#pragma once

#include "DataStructures/DominatorTree.hpp"
#include "DataStructures/Tree.hpp"
#include "IR/BasicBlock.hpp"

namespace koda {

class Compiler;

class AnalysisBase {
  bool m_is_ready = false;

public:
  AnalysisBase() : m_is_ready(false) {}
  AnalysisBase(bool is_ready) : m_is_ready(is_ready) {}
  AnalysisBase(const AnalysisBase &) = delete;
  AnalysisBase(AnalysisBase &&) = delete;
  AnalysisBase &operator=(const AnalysisBase &) = delete;
  AnalysisBase &operator=(AnalysisBase &&) = delete;
  virtual ~AnalysisBase() = default;

  bool is_ready() const { return m_is_ready; }
  void set_ready(bool Val) { m_is_ready = Val; }
};

class RPOAnalysis final : public AnalysisBase {
  std::vector<bbid_t> m_rpo;

public:
  virtual ~RPOAnalysis() = default;
  void run(ProgramGraph &graph);
  std::vector<bbid_t> &blocks() { return m_rpo; }
  auto begin() { return m_rpo.begin(); }
  auto end() { return m_rpo.end(); }
};

struct DomsTreeAnalysis : public AnalysisBase {
public:
  using DomsTree = DominatorTree<BasicBlock *>;

private:
  DomsTree m_dom_tree{nullptr};

public:
  virtual ~DomsTreeAnalysis() = default;
  void run(ProgramGraph &graph);
  DomsTree &get() { return m_dom_tree; }
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

  void add_block(BasicBlock *bb) { m_blocks.push_back(bb); }

  iterator begin() { return m_blocks.begin(); }

  iterator end() { return m_blocks.end(); }
};

struct LoopTreeAnalysis : public AnalysisBase {
public:
  using loop_id_t = bbid_t;
  using LoopTree = Tree<loop_id_t, LoopInfo>;
  constexpr static loop_id_t ROOT_LOOP_ID = -1;
  constexpr static loop_id_t NONE_LOOP_ID = -2;

private:
  LoopTree m_loop_tree{NONE_LOOP_ID};

public:
  virtual ~LoopTreeAnalysis() = default;
  void run(Compiler &comp);
  void run(ProgramGraph &graph, DomsTreeAnalysis::DomsTree &doms);
  LoopTree &get() { return m_loop_tree; }
};


} // namespace koda