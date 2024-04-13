#pragma once

#include <Core/Analyses.hpp>
#include <Core/Passes.hpp>
#include <DataStructures/DominatorTree.hpp>
#include <IR/ProgramGraph.hpp>

namespace koda {

class Compiler final {
  std::vector<std::unique_ptr<PassI>> m_passes;

  ProgramGraph m_graph;

  RPOAnalysis m_rpo;

  DomsTreeAnalysis m_dom_tree;

  LoopTreeAnalysis m_loop_tree;

public:
  Compiler();
  Compiler(const Compiler &Compiler) = delete;
  Compiler(Compiler &&) = delete;
  Compiler &operator=(const Compiler &Compiler) = delete;
  Compiler &operator=(Compiler &&Compiler) = delete;

  ProgramGraph &graph() { return m_graph; }

  template <typename Analysis> Analysis &get();

  template <typename Analysis, typename... Args>
  Analysis &get_or_create(Args &&...args) {
    static_assert(std::is_base_of<AnalysisBase, Analysis>::value,
                  "Analysis expected");
    auto &&instance = get<Analysis>();
    if (!instance.is_ready()) {
      instance.run(std::forward<Args>(args)...);
      instance.set_ready(true);
    }
    return instance;
  }

  template <typename Pass, typename... Args>
  void register_pass(Args &&...args) {
    static_assert(std::is_base_of<PassI, Pass>::value,
                  "Pass must implement interface");
    m_passes.emplace_back(std::forward<Args>(args)...);
  }

  void run_all_passes(ProgramGraph &graph);
};

template <> inline RPOAnalysis &Compiler::get<RPOAnalysis>() { return m_rpo; }
template <> inline DomsTreeAnalysis &Compiler::get<DomsTreeAnalysis>() {
  return m_dom_tree;
}
template <> inline LoopTreeAnalysis &Compiler::get<LoopTreeAnalysis>() {
  return m_loop_tree;
}

} // namespace koda