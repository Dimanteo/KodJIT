#pragma once

#include "Core/LoopInfo.hpp"
#include "DataStructures/DominatorTree.hpp"
#include "DataStructures/Tree.hpp"
#include "IR/BasicBlock.hpp"

#include <set>
#include <unordered_map>
#include <optional>

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
  void set_ready(bool val) { m_is_ready = val; }
};

class RPOAnalysis final : public AnalysisBase {
  std::vector<bbid_t> m_rpo;

public:
  virtual ~RPOAnalysis() = default;
  void run(ProgramGraph &graph);
  void run(Compiler &comp);
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
  void run(Compiler &comp);
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
  auto rbegin() const { return m_linear_order.rbegin(); }
  auto rend() const { return m_linear_order.rend(); }
};

class Liveness : public AnalysisBase {
  using LiveRange = std::pair<size_t, size_t>;
  using RangeMap = std::vector<LiveRange>;

  RangeMap m_live_ranges;

  void extend_liverange(instid_t inst, const LiveRange &range) {
    auto &liverange = m_live_ranges[inst];
    liverange.first = std::min(liverange.first, range.first);
    liverange.second = std::max(liverange.second, range.second);
  };

public:
  virtual ~Liveness() = default;

  void run(Compiler &compiler);

  LiveRange get_live_range(instid_t iid) const { return m_live_ranges[iid]; }
};

namespace _detailRegalloc {

struct Interval {
  instid_t inst;
  size_t begin;
  size_t end;
};

inline bool operator<(const Interval &lhs, const Interval &rhs) {
  if (lhs.end == rhs.end) {
    if (lhs.begin == rhs.begin) {
      return lhs.inst < rhs.inst;
    }
    return lhs.begin < rhs.begin;
  }
  return lhs.end < rhs.end;
}

};

class RegAlloc : public AnalysisBase {
  using Interval = _detailRegalloc::Interval;
  using locid_t = int;

  constexpr static locid_t INVALID_REG = -1;

  size_t m_regnum;

  locid_t m_slot_num = 0;

  std::vector<locid_t> m_free_pool;

  std::set<Interval> m_active;

  std::vector<locid_t> m_regmap;

  std::unordered_map<instid_t, locid_t> m_slotmap;

  void expire_old_intervals(const Interval &inter);

  void spill_at_interval(const Interval &inter);

  locid_t alloc_stack_slot() { return m_slot_num++; }

  bool is_spilled(instid_t inst) const {
    return m_slotmap.find(inst) != m_slotmap.end();
  }

  void reset(Compiler &compiler);

public:
  struct Location {
    locid_t location;
    bool is_stack;
  };

  virtual ~RegAlloc() = default;

  void run(Compiler &compiler);

  std::optional<Location> get_location(instid_t inst) {
    auto slot = m_slotmap.find(inst);
    if (slot == m_slotmap.end()) {
      locid_t loc = m_regmap[inst];
      if (loc == INVALID_REG) {
        return std::nullopt;
      }
      return Location{loc, false};
    }
    return Location{slot->second, true};
  }
};

} // namespace koda