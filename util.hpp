#pragma once
#include "value.hpp"
#include "vm.hpp"

#include <utility>

namespace cxxlisp {

/**
 * Convert from Value to T.
 *
 * Because variadic template need functional template.
 */
template <typename T> inline T val_as(Value v);

template <> inline vint_t val_as<vint_t>(Value v) { return v.AsNumber(); }
template <> inline bool val_as<bool>(Value v) { return v.Truthy(); }
template <> inline Atom val_as<Atom>(Value v) { return v.AsAtom(); }
template <> inline Cell &val_as<Cell &>(Value v) { return v.AsCell(); }
template <> inline const std::string &val_as<const std::string &>(Value v) {
  return v.AsString();
}
template <> inline Procedure &val_as<Procedure &>(Value v) {
  return v.AsProcedure();
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

/*
 * Create tuple from array and types.
 * See: https://stackoverflow.com/questions/15014096
 */
template <typename... T, std::size_t... Is>
std::tuple<T...> make_tuple_vals(Value *v, std::index_sequence<Is...>) {
  return std::make_tuple(val_as<T>(v[Is])...);
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
  return make_tuple_vals<T...>(vals, std::make_index_sequence<sizeof...(T)>());
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
  return make_tuple_vals<T...>(vals, std::make_index_sequence<sizeof...(T)>());
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

  template <std::size_t... Is>
  Value call(Ctx &ctx, Value *vals, std::index_sequence<Is...>) {
    return f_(ctx, val_as<T>(vals[Is])...);
  }

public:
  static const int ARITY = sizeof...(T);

  ProcCaller(FuncType f) : f_(f) {}

  Value operator()(Ctx &ctx, Value args) {
    Value vals[ARITY];
    spread(ARITY, vals, args);
    return call(ctx, vals, std::make_index_sequence<sizeof...(T)>());
  }

  int Arity() { return ARITY; }
};

template <typename T> Procedure *make_procedure(T func) {
  auto pc = ProcCaller(func);
  return new Procedure(pc.Arity(), pc);
}

template <typename T>
void add_proc(VM &vm, bool is_macro, const char *id, T f) {
  auto *proc = make_procedure(f);
  proc->SetName(id);
  proc->SetIsMacro(is_macro);
  vm.RootEnv().Define(vm.Intern(id), proc);
}

void add_proc_varg(VM &vm, bool is_macro, const char *id,
                   Value (*f)(Ctx &, Value));

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
inline ListIterator end([[maybe_unused]] Value &v) { return ListIterator(); }

Value run(VM &vm, std::string_view src);

// pretty_print.cpp
const int PRETTY_PRINT_DEFAULT_LEN = 256;
std::ostream &pretty_print(std::ostream &os, const VM &vm, Value v,
                           int len = PRETTY_PRINT_DEFAULT_LEN);

} // namespace cxxlisp
