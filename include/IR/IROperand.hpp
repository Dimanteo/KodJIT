#pragma once

#include <IR/IRTypes.hpp>

namespace koda {

class BasicBlock;
class Instruction;

enum OperandType { TYPE_INVALID = 0, NONE, BOOLEAN, BYTE, INTEGER, FLOAT, STRING, LABEL };

inline constexpr const char *operandTypeToStr(OperandType op) {
  switch (op) {
    case TYPE_INVALID:
      return "invalid";
    case NONE:
      return "none";
    case BOOLEAN:
      return "bool";
    case BYTE:
      return "byte";
    case INTEGER:
      return "int";
    case FLOAT:
      return "float";
    case STRING:
      return "str";
    case LABEL:
      return "lbl";
    default:
      return "Unknown_type";
  }
  return "Unknown_type";
}

struct IOperand {
  virtual OperandType getType() const = 0;
};

}; // namespace koda
