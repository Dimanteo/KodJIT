#pragma once

#include <DataStructures/Tree.hpp>
#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>

#include <optional>

namespace koda {

class Compiler;
class IRBuilder;

// Base class for all passes
struct PassI {
  virtual ~PassI() = default;
  virtual void run(Compiler &compiler) = 0;
};

// Not a DCE. Used to clean redundant instructions left after other passes.
class RmUnused : public PassI {
public:
  virtual ~RmUnused() = default;

  void run(Compiler &comp) override;
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
  std::optional<Instruction *> peephole_and(IRBuilder &builder,
                                            Instruction *inst);

  std::optional<Instruction *> peephole_sub(IRBuilder &builder,
                                            Instruction *inst);

  static bool is_const_eq(Instruction *inst, int64_t value);

public:
  virtual ~Peephole() = default;

  void run(Compiler &compiler) override;
};

} // namespace koda