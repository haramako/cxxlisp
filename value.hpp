#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace cxxlisp {

class VM;
class CustomObject;
class Cell;
class StringValue;

class Atom {
  int id_;

public:
  Atom(int id) : id_(id) {}
  int Id() { return id_; }
  const std::string &ToString();
};

enum class ValueType { NIL, NUMBER, ATOM, CELL, STRING, CUSTOM_OBJECT };

/**
 * Lisp value.
 */
class Value {
  ValueType type_;
  uintptr_t i_;
  void *p_;

public:
  Value() : type_(ValueType::NIL) {}
  Value(int v) : type_(ValueType::NUMBER), i_(v) {}
  Value(Atom v) : type_(ValueType::ATOM), i_(v.Id()) {}
  Value(Cell *v) : type_(ValueType::CELL), p_(v) {}
  Value(const std::string &v);

  ValueType Type() const { return type_; }

  bool IsNil() const { return type_ == ValueType::NIL; }
  bool IsNumber() const { return type_ == ValueType::NUMBER; }
  bool IsAtom() const { return type_ == ValueType::ATOM; }
  bool IsCell() const { return type_ == ValueType::CELL; }
  bool IsString() const { return type_ == ValueType::STRING; }

  int AsNumber() const { return (int)i_; }
  Atom AsAtom() const { return Atom((int)i_); }
  const Cell &AsCell() { return *((Cell *)p_); }
  const StringValue &AsStringValue() { return *((StringValue *)p_); }
  const std::string &AsString() const;
  const CustomObject &AsCustomObject() { return *((CustomObject *)p_); }

  friend bool operator==(const Value &, const Value &);
};

bool operator==(const Value &a, const Value &b);
inline bool operator!=(const Value &a, const Value &b) { return !(a == b); }

/**
 * Base class of lisp object.
 */
class CustomObject {
public:
  virtual std::string str() = 0;
};

/**
 * Pair object.
 */
class Cell final {
  Value car_, cdr_;

public:
  std::string str();
};

class StringValue final {
  std::string v_;

public:
  StringValue(const std::string &v) : v_(v) {}
  std::string str() const { return v_; };
  const std::string &Ref() const { return v_; };
};

extern Value NIL;

} // namespace cxxlisp