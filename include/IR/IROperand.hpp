#pragma once

#include <IR/IRTypes.hpp>

namespace koda {

class BasicBlock;
class Instruction;

enum OperandType { TYPE_INVALID = 0, NONE, BOOLEAN, BYTE, INTEGER, FLOAT, STRING, LABEL };

// Must be aligned with OperanType enum.
// Cringe.
constexpr const char *OperandTypeToStr[] = {"invalid", "none", "bool", "byte", "int", "float", "str", "lbl"};

struct IOperand {
  virtual OperandType getType() const = 0;
};

}; // namespace koda
