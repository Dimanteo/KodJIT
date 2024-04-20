#pragma once

#include "Core/LoopInfo.hpp"
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
  auto begin() const { return m_rpo.begin(); }
  auto end() const { return m_rpo.end(); }
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

struct LoopTreeAnalysis : public AnalysisBase {
public:
  using loop_id_t = LoopInfo::loop_id_t;
  using LoopTree = Tree<loop_id_t, LoopInfo>;

private:
  LoopTree m_loop_tree{LoopInfo::INVALID_LOOP_ID};

public:
  virtual ~LoopTreeAnalysis() = default;
  void run(Compiler &comp);
  void run(ProgramGraph &graph, DomsTreeAnalysis::DomsTree &doms);
  LoopTree &get() { return m_loop_tree; }
  const LoopTree &get() const { return m_loop_tree; }
  const LoopInfo &get_loop(const BasicBlock &bb) const;
};

class LinearOrder : public AnalysisBase {

  std::vector<BasicBlock *> m_linear_order;

  void linearize_graph(Compiler &comp);
  void linearize_loop(const BasicBlock &header, const LoopTreeAnalysis &loops,
                      std::vector<bool> &visited);

public:
  virtual ~LinearOrder() = default;
  void run(Compiler &comp);
  auto begin() const { return m_linear_order.begin(); }
  auto end() const { return m_linear_order.end(); }
};

} // namespace koda