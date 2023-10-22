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
  virtual bool isTrackingUsers() const = 0;
  template <class T> T *cast_to() {
    static_assert(std::is_base_of<IOperand, T>::value);
    // Not using dynamic cast here to avoid runtime checks.
    return reinterpret_cast<T *>(this);
  }
};

struct IPhiOperand : public IOperand {
  virtual BasicBlock *getSrcBB() const = 0;
};

class ProgParam : public IOperand {
  OperandType m_type;

public:
  ProgParam(OperandType type) : m_type(type) {}
  virtual ~ProgParam() = default;
  OperandType getType() const override { return m_type; }
  bool isTrackingUsers() const override { return false; }
};

struct IntConstOperand : public IOperand {
  uint64_t m_value;

public:
  IntConstOperand(uint64_t val) : m_value(val) {}
  virtual ~IntConstOperand() = default;
  OperandType getType() const override { return OperandType::INTEGER; }
  uint64_t getValue() const { return m_value; }
  bool isTrackingUsers() const override { return false; }
};

}; // namespace koda
