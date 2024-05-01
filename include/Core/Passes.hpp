#pragma once

#include <DataStructures/Tree.hpp>
#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>

namespace koda {

class Compiler;

// Base class for all passes
struct PassI {
  virtual ~PassI() = default;
  virtual void run(Compiler &compiler) = 0;
};

class ConstantFolding : public PassI {
public:
  using ConstInst = LoadConstant<int64_t>;

private:
  static bool is_computable(const Instruction &inst);

  static bool has_const_input(const Instruction &inst);

  int64_t fold(const Instruction &act);

  using Evaluator = std::function<int64_t(const Instruction &)>;

  std::unordered_map<InstOpcode, Evaluator> m_evaluators;

public:
  virtual ~ConstantFolding() = default;

  ConstantFolding();

  void run(Compiler &compiler) override;
};

class Peephole : public PassI {
  void peephole_and(Compiler &comp, BasicBlock &bb);

public:
  virtual ~Peephole() = default;

  void run(Compiler &compiler) override;
};

} // namespace koda