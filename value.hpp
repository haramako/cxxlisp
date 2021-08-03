#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
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

/**
 * Base class of lisp exceptions.
 */
class LispException : public std::exception {
  std::string msg_;

public:
  LispException(const char *msg) : msg_(msg) {}
  LispException(const std::string_view msg) : msg_(msg) {}
  const char *what() const noexcept override { return msg_.c_str(); }
};

/**
 * Base class of lisp exceptions.
 */
class BUG : public LispException {
public:
  BUG() : LispException("BUG") {}
};

class Atom {
  atom_id_t id_;

public:
  Atom(atom_id_t id) : id_(id) {}
  atom_id_t Id() { return id_; }
  const std::string &ToString();
};

enum class ValueType : uint8_t {
  NIL,
  SPECIAL,
  NUMBER,
  ATOM,
  CELL,
  STRING,
  PROCEDURE,
  CUSTOM_OBJECT,
};

extern const char *VALUE_TYPE_NAMES[];

inline const char *to_str(ValueType v) { return VALUE_TYPE_NAMES[(int)v]; }

inline std::ostream &operator<<(std::ostream &s, ValueType v) {
  return s << to_str(v);
}

/**
 * Lisp value.
 */
class Value {
  ValueType type_;
  uintptr_t i_;
  template <class T> T &ref() { return *reinterpret_cast<T *>(i_); }
  template <class T> const T &ref() const { return *reinterpret_cast<T *>(i_); }

  void chk(ValueType vt) const {
    if (type_ != vt) {
      std::stringstream s;
      s << "Value is not " << vt << ", but " << type_ << ".";
      throw LispException(s.str());
    }
  }

  Value(ValueType vt, uintptr_t v) : type_(vt), i_(v) {}

public:
  Value() : type_(ValueType::NIL) {}
  Value(int v) : type_(ValueType::NUMBER), i_(v) {}
  Value(Atom v) : type_(ValueType::ATOM), i_(v.Id()) {}
  Value(Cell *v) : type_(ValueType::CELL), i_((uintptr_t)v) { assert(v); }
  Value(const std::string &v);
  Value(const char *v) : Value(std::string(v)) { assert(v); }
  Value(Procedure *v) : type_(ValueType::PROCEDURE), i_((uintptr_t)v) {
    assert(v);
  }

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

  bool Truthy() const { return !Falsy(); }
  bool Falsy() const { return type_ == ValueType::NIL || IsF(); }

  int AsNumber() const {
    chk(ValueType::NUMBER);
    return (int)i_;
  }
  const std::string &AsSpecial() const {
    chk(ValueType::SPECIAL);
    return ref<std::string>();
  }
  Atom AsAtom() const {
    chk(ValueType::ATOM);
    return Atom((int)i_);
  }
  Cell &AsCell() {
    chk(ValueType::CELL);
    return ref<Cell>();
  }
  const Cell &AsCell() const {
    chk(ValueType::CELL);
    return ref<Cell>();
  }
  const StringValue &AsStringValue() {
    chk(ValueType::STRING);
    return ref<StringValue>();
  }
  const std::string &AsString() const;
  const Procedure &AsProcedure() const {
    chk(ValueType::PROCEDURE);
    return ref<Procedure>();
  }
  const CustomObject &AsCustomObject() {
    chk(ValueType::CUSTOM_OBJECT);
    return ref<CustomObject>();
  }

  const std::string ToString(const VM *vm = nullptr) const;
  const std::string ToString(const VM &vm) const;
  friend bool operator==(const Value &, const Value &);

  static Value CreateSpecial(std::string_view name) {
    return Value(ValueType::SPECIAL,
                 reinterpret_cast<uintptr_t>(new std::string(name)));
  }
};

inline std::ostream &operator<<(std::ostream &s, const Value &value) {
  s << value.ToString();
  return s;
}

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
