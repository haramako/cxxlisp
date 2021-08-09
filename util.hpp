#pragma once

#include "value.hpp"

namespace cxxlisp {

/**
 * Convert from Value to T.
 *
 * Because variadic template need functional template.
 */
template <typename T> inline T val_as(Value v);

template <> inline vint_t val_as<vint_t>(Value v) { return v.AsNumber(); }
template <> inline Atom val_as<Atom>(Value v) { return v.AsAtom(); }
template <> inline const std::string &val_as<const std::string &>(Value v) {
  return v.AsString();
}
template <> inline Value val_as<Value>(Value v) { return v; }

inline Value car(Value v) { return v.AsCell().Car; }
inline Value cdr(Value v) { return v.AsCell().Cdr; }

/**
 * cons.
 *
 * Usage:
 * cons(a, b, c, d) => Value of (a b c . d)
 */
inline Value cons(Value v) { return v; }
template <typename... REST> inline Value cons(Value v, REST... rest) {
  return new Cell(v, cons(rest...));
}

/**
 * list.
 *
 * Usage:
 * list(a, b, c, d) => Value of (a b c d)
 */
inline Value list() { return NIL; }
template <typename... REST> inline Value list(Value v, REST... rest) {
  return new Cell(v, list(rest...));
}

/**
 * uncons.
 *
 * Usage:
 *   uncons(cons(1, 2, 3)); => std::tuple(1, 2, 3);
 *
 */
template <typename... T> std::tuple<T...> uncons(Value pair) {
  const int n = sizeof...(T);
  Value vals[n];
  for (int i = 0; i < n; i++) {
    vals[i] = pair.AsCell().Car;
    pair = pair.AsCell().Cdr;
  }
  int i = n - 1;
  return std::make_tuple(val_as<T>(vals[i--])...);
}

/**
 * uncons.
 *
 * Usage:
 *   uncons_rest<vint_t,vint_t,Value>(cons(1, 2, 3), 4); => std::tuple(1, 2,
 * cons(3,4));
 *
 */
template <typename... T> std::tuple<T...> uncons_rest(Value pair) {
  const int n = sizeof...(T);
  Value vals[n];
  for (int i = 0; i < n; i++) {
    if (i == n - 1) {
      vals[i] = pair;
    } else {
      vals[i] = pair.AsCell().Car;
      pair = pair.AsCell().Cdr;
    }
  }
  int i = n - 1;
  return std::make_tuple(val_as<T>(vals[i--])...);
}

/**
 * Functional object of C++ function.
 *
 * Convert C++ function to Value(VM&,Value) function.
 *
 * Usage::
 *   Value f(VM &vm, vint_t arg0, const string &arg1);
 *
 *   std::functional<Value(VM&,Value)> proc = ProcCaller(f);
 *
 *   proc.Arity(); // => 2
 *   proc.Call(vm, list(1,"hoge")); // => f(vm, t, "hoge")
 */
template <typename... T> class ProcCaller {
  using FuncType = Value (*)(Ctx &, T...);
  FuncType f_;

public:
  static const int ARITY = sizeof...(T);

  ProcCaller(FuncType f) : f_(f) {}

  Value operator()(Ctx &vm, Value args) {
    Value vals[ARITY];
    spread(ARITY, vals, args);
    int i = ARITY;
    return f_(vm, val_as<T>(vals[--i])...);
  }

  int Arity() { return ARITY; }
};

template <typename T> Procedure *make_procedure(T func) {
  return new Procedure(ProcCaller(func).Arity(), ProcCaller(func));
}

/**
 * Iterator of cons list.
 *
 * Usage:
 *   Value li = list(1,2,3,4);
 *   for(auto it: li){
 *     ....
 *   }
 */
class ListIterator {
  Cell *v_;

public:
  ListIterator() : v_(nullptr) {}

  ListIterator(Value &v) {
    if (v.IsNil()) {
      v_ = nullptr;
    } else {
      v_ = &v.AsCell();
    }
  }

  Value operator*() { return v_->Car; }

  ListIterator &operator++() {
    if (v_->Cdr.IsNil()) {
      v_ = nullptr;
    } else {
      v_ = &v_->Cdr.AsCell();
    }
    return *this;
  }

  bool operator!=(const ListIterator &other) { return v_ != other.v_; }
};

inline ListIterator begin(Value &v) { return ListIterator(v); }
inline ListIterator end(Value &v) { return ListIterator(); }

} // namespace cxxlisp
