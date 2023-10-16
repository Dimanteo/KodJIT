#pragma once

#include <IR/IRTypes.hpp>

namespace koda {

class BasicBlock;
class Instruction;

enum OperandType { TYPE_INVALID = 0, NONE, BOOLEAN, BYTE, INTEGER, FLOAT, STRING, LABEL };

// Must be aligned with OperanType enum.
// Cringe.
constexpr const char *OperandTypeToStr[] = {"Invalid", "None", "Bool", "Int", "Float", "Str", "Lbl"};

struct IOperand {
  virtual OperandType getType() const = 0;
  virtual bool isTrackingUsers() const { return false; };
  template <class T> T *cast_to() {
    static_assert(std::is_base_of<IOperand, T>::value);
    // Not using dynamic cast here to avoid runtime checks.
    return reinterpret_cast<T *>(this);
  }
};

struct IInstructionOperand : public IOperand {
  virtual Instruction *getInstruction() = 0;
  bool isTrackingUsers() const override { return true; }
  virtual void addUser(Instruction *);
};

struct IPhiOperand : public IOperand {
  virtual BasicBlock *getSrcBB() const = 0;
};

}; // namespace koda
