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

using FuncAny = Value (*)(VM &, Value);
using Func0 = Value (*)(VM &);
using Func1 = Value (*)(VM &, Value);
using Func2 = Value (*)(VM &, Value, Value);
using Func3 = Value (*)(VM &, Value, Value);

template <int N> struct FuncN { const int a = N; };
template <> struct FuncN<-1> { using Type = FuncAny; };
template <> struct FuncN<0> { using Type = Func0; };
template <> struct FuncN<1> { using Type = Func1; };
template <> struct FuncN<2> { using Type = Func2; };
template <> struct FuncN<3> { using Type = Func3; };

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

  template <typename T> T As() {}
  const std::string ToString(const VM *vm = nullptr) const;
  const std::string ToString(const VM &vm) const;
  friend bool operator==(const Value &, const Value &);

  static Value CreateSpecial(std::string_view name) {
    return Value(ValueType::SPECIAL,
                 reinterpret_cast<uintptr_t>(new std::string(name)));
  }
};

template <> inline vint_t Value::As<vint_t>() { return (vint_t)AsNumber(); }
template <> inline const std::string &Value::As<const std::string &>() {
  return AsString();
}
template <> inline Value Value::As<Value>() { return *this; }

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

class StringValue final {
  std::string v_;

public:
  StringValue(const std::string &v) : v_(v) {}
  std::string str() const { return v_; };
  const std::string &Ref() const { return v_; };
};

/**
 * get_args.
 *
 * Usage:
 *   Value args[2];
 *   get_args(args, head); // head = (1 2);
 */
template <int N> inline void get_args(Value (&args)[N], Value head) {
  get_args_<N>(args, head);
}

template <int N> inline void get_args_(Value *args, Value head) {
  *args = head.AsCell().Car;
  get_args_<N - 1>(args + 1, head.AsCell().Cdr);
}

template <> inline void get_args_<0>(Value *args, Value head) {}

template <typename T> struct AlwaysValue {
  using t = Value;
  static T AS(Value v) { return v.As<T>(); }
};

inline Value cons() { return NIL; }
template <typename... REST> inline Value cons(Value v, REST... rest) {
  return new Cell(v, cons(rest...));
}

template <typename T> inline T from_val(Value &head) {
  T r = head.AsCell().Car.As<T>();
  head = head.AsCell().Cdr;
  return r;
}

template <int I, int N, typename... T>
void uncons_(std::tuple<T...> &r, Value head) {
  if (I < N) {
    set<I>(r, head.AsCell().Car);
    uncons<I + 1, N>(r, head.AsCell().Cdr);
  }
}

template <typename... T> std::tuple<T...> uncons(Value pair) {
  const int n = sizeof...(T);
  Value vals[n];
  for (int i = 0; i < n; i++) {
    vals[i] = pair.AsCell().Car;
    pair = pair.AsCell().Cdr;
  }
  int i = n - 1;
  return std::make_tuple(AlwaysValue<T>::AS(vals[i--])...);
}

inline void spread(int n, Value *vals, Value head) {
  for (int i = 0; i < n; i++) {
    vals[i] = head.AsCell().Car;
    head = head.AsCell().Cdr;
  }
}

template <typename... T> struct caller {
  using FuncType = Value (*)(VM &, T...);
  static const int n = sizeof...(T);
  FuncType f_;
  caller(FuncType f) : f_(f) {}
  Value operator()(VM &vm, Value args) {
    Value vals[n];
    spread(n, vals, args);
    int i = n;
    return f_(vm, AlwaysValue<T>::AS(vals[--i])...);
  }
};

class Procedure {
  int arity_;

  void check(int arity) const;

public:
  std::function<Value(VM &, Value)> func_;
  template <typename T>
  Procedure(T func) : arity_(decltype(caller(func))::n), func_(caller(func)) {}

  int Arity() const { return arity_; }
};

} // namespace cxxlisp
