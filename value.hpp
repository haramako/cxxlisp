#pragma once
#include <cassert>
#include <cstdint>
#include <functional>
#include <gc_cpp.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "errors.hpp"

namespace cxxlisp {

#if 0
class gc {};
class gc_cleanup {};
#endif

using atom_id_t = int;
using vint_t = int;

class VM;
class Ctx;
class CustomObject;
class Cell;
class StringValue;
class Procedure;
class Value;

extern Value NIL;
extern Value BOOL_F;
extern Value BOOL_T;
extern Value UNDEF;
extern Value SYM_QUOTE;
extern Value SYM_QUASIQUOTE;
extern Value SYM_UNQUOTE;
extern Value SYM_LAMBDA;
extern Value SYM_UNQUOTE_SPLICING;

class Atom {
  atom_id_t id_;

public:
  explicit Atom(atom_id_t id) : id_(id) {}
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

enum class SpecialType {
  IF,
  QUOTE,
  LAMBDA,
  DEFINE,
  BEGIN,
};

enum class SpecialForm {
  BEGIN = 0,
  DEFINE,
  IF,
  LAMBDA,
  QUOTE,
  QUASIQUOTE,
  UNQUOTE,
  LOOP,
  SET_EX,
  UNQUOTE_SPLICING,
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
  Value(bool v) : Value(v ? BOOL_T : BOOL_F) {}
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

  bool IsT() const { return *this == BOOL_T; }
  bool IsF() const { return *this == BOOL_F; }

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
  const Cell &AsCell() const { return AsCell(); }

  const StringValue &AsStringValue() {
    chk(ValueType::STRING);
    return ref<StringValue>();
  }
  const std::string &AsString() const;

  Procedure &AsProcedure() {
    chk(ValueType::PROCEDURE);
    return ref<Procedure>();
  }
  const Procedure &AsProcedure() const { return AsProcedure(); }

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

  static Value CreateSpecialForm(SpecialForm sf) {
    return Value(ValueType::ATOM, (uintptr_t)sf);
  }
};

inline std::ostream &operator<<(std::ostream &s, const Value &value) {
  return s << value.ToString();
}

bool operator==(const Value &a, const Value &b);
inline bool operator!=(const Value &a, const Value &b) { return !(a == b); }

/**
 * Base class of lisp object.
 */
class CustomObject : public gc_cleanup {
public:
  virtual std::string str() = 0;
};

/**
 * Pair object.
 */
class Cell final : public gc {
public:
  std::string str();
  Value Car, Cdr;
  Cell() : Car(), Cdr() {}
  Cell(Value car, Value cdr) : Car(car), Cdr(cdr) {}

  Cell(const Cell &) = delete;
  Cell &operator=(const Cell &) = delete;
};

/**
 * StringValue
 */
class StringValue final : public gc_cleanup {
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
class Procedure : public gc_cleanup {
  using func_t = std::function<Value(Ctx &, Value)>;

  bool isNative_ = false;
  int arity_ = 0;
  func_t func_ = nullptr;
  Value params_;
  Value body_;
  bool isMacro_ = false;

  std::string name_;

public:
  Procedure(int arity, func_t func)
      : isNative_(true), arity_(arity), func_(func) {}
  Procedure(Value params, Value body)
      : isNative_(false), params_(params), body_(body) {}

  bool IsNative() const { return isNative_; }
  int Arity() const { return arity_; }
  func_t Func() const { return func_; }
  Value Params() const { return params_; }
  Value Body() const { return body_; }
  const std::string &Name() const { return name_; }
  void SetName(std::string_view v) { name_ = v; }
  bool IsMacro() const { return isMacro_; }
  void SetIsMacro(bool v) { isMacro_ = v; }
};

} // namespace cxxlisp
