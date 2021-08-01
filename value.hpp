#pragma once
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <variant>

namespace cxxlisp {

using atom_id_t = int;
using vint_t = int;

class VM;
class CustomObject;
class Cell;
class StringValue;
class Procedure;
class Value;

extern Value NIL;
extern Value BOOL_F;
extern Value BOOL_T;
extern Value SYM_QUOTE;
extern Value SYM_QUASIQUOTE;
extern Value SYM_UNQUOTE;

class Atom {
  atom_id_t id_;

public:
  Atom(atom_id_t id) : id_(id) {}
  atom_id_t Id() { return id_; }
  const std::string &ToString();
};

enum class ValueType {
  NIL,
  SPECIAL,
  NUMBER,
  ATOM,
  CELL,
  STRING,
  PROCEDURE,
  CUSTOM_OBJECT
};

/**
 * Lisp value.
 */
class Value {
  ValueType type_;
  uintptr_t i_;
  template <class T> T &ref() { return *reinterpret_cast<T *>(i_); }
  template <class T> const T &ref() const { return *reinterpret_cast<T *>(i_); }

  Value(ValueType vt, uintptr_t v) : type_(vt), i_(v) {}

public:
  Value() : type_(ValueType::NIL) {}
  Value(int v) : type_(ValueType::NUMBER), i_(v) {}
  Value(Atom v) : type_(ValueType::ATOM), i_(v.Id()) {}
  Value(Cell *v) : type_(ValueType::CELL), i_((uintptr_t)v) {}
  Value(const std::string &v);
  Value(const char *v) : Value(std::string(v)) {}
  Value(Procedure *v) : type_(ValueType::PROCEDURE), i_((uintptr_t)v) {}

  ValueType Type() const { return type_; }

  bool IsNil() const { return type_ == ValueType::NIL; }
  bool IsSpecial() const { return type_ == ValueType::SPECIAL; }
  bool IsNumber() const { return type_ == ValueType::NUMBER; }
  bool IsAtom() const { return type_ == ValueType::ATOM; }
  bool IsCell() const { return type_ == ValueType::CELL; }
  bool IsString() const { return type_ == ValueType::STRING; }
  bool IsProcedure() const { return type_ == ValueType::PROCEDURE; }

  bool IsT() const { return this == &BOOL_T; }
  bool IsF() const { return this == &BOOL_F; }

  int AsNumber() const { return (int)i_; }
  const std::string &AsSpecial() const { return ref<std::string>(); }
  Atom AsAtom() const { return Atom((int)i_); }
  Cell &AsCell() { return ref<Cell>(); }
  const Cell &AsCell() const { return ref<Cell>(); }
  const StringValue &AsStringValue() { return ref<StringValue>(); }
  const std::string &AsString() const;
  const Procedure &AsProcedure() const { return ref<Procedure>(); }
  const CustomObject &AsCustomObject() { return ref<CustomObject>(); }

  const std::string ToString(const VM *vm = nullptr) const;
  const std::string ToString(const VM &vm) const;
  friend bool operator==(const Value &, const Value &);

  static Value CreateSpecial(std::string_view name) {
    return Value(ValueType::SPECIAL,
                 reinterpret_cast<uintptr_t>(new std::string(name)));
  }
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
public:
  std::string str();
  Value Car, Cdr;
  Cell() : Car(), Cdr() {}
  Cell(Value car, Value cdr) : Car(car), Cdr(cdr) {}
};

/**
 * StringValue
 */
class StringValue final {
  std::string v_;

public:
  StringValue(const std::string &v) : v_(v) {}
  std::string str() const { return v_; };
  const std::string &Ref() const { return v_; };
};

inline void spread(int n, Value *vals, Value head) {
  for (int i = 0; i < n; i++) {
    vals[i] = head.AsCell().Car;
    head = head.AsCell().Cdr;
  }
}

/**
 * Procedure
 */
class Procedure {
  int arity_;
  std::function<Value(VM &, Value)> func_;

  void check(int arity) const;

public:
  Procedure(int arity, std::function<Value(VM &, Value)> func)
      : arity_(arity), func_(func) {}

  int Arity() const { return arity_; }
  Value Call(VM &vm, Value args) { return func_(vm, args); }
};

} // namespace cxxlisp
