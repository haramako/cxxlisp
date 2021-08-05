#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

template <typename T> T fold(Value list, T (*f)(T, T)) {
  Value head = car(list);
  Value rest = cdr(list);
  T r = val_as<T>(head);
  for (auto v : rest) {
    r = f(r, val_as<T>(v));
    rest = cdr(rest);
  }
  return r;
}

template <typename T> T foldc(Value list, T (*f)(const T &, const T &)) {
  Value head = car(list);
  Value rest = cdr(list);
  T r = val_as<const T &>(head);
  for (auto v : rest) {
    r = f(r, val_as<const T &>(v));
    rest = cdr(rest);
  }
  return r;
}

static Value add(Ctx &ctx, Value args) {
  Value head = car(args);
  if (head.Type() == ValueType::NUMBER) {
    return fold<vint_t>(args, [](vint_t a, vint_t b) { return a + b; });
  } else if (head.Type() == ValueType::STRING) {
    return foldc<string>(
        args, [](const string &a, const string &b) { return a + b; });
  } else {
    throw BUG();
  }
}

static Value sub(Ctx &ctx, Value args) {
  Value head = car(args);
  Value rest = cdr(args);
  if (head.Type() == ValueType::NUMBER) {
    vint_t r = head.AsNumber();
    for (auto v : rest) {
      r -= v.AsNumber();
      rest = cdr(rest);
    }
    return r;
  } else {
    throw BUG();
  }
}

static Value list_(Ctx &ctx, Value args) { return args; }

static Value cons_(Ctx &ctx, Value v1, Value v2) { return new Cell(v1, v2); }

#define F(id, f) env.Set(vm.Intern(id), make_procedure(f));
#define FVARG(id, f) env.Set(vm.Intern(id), new Procedure(-1, f));

void init_func(VM &vm) {
  auto &env = vm.RootEnv();
  FVARG("+", add);
  FVARG("-", sub);
  F("cons", cons_);
  FVARG("list", list_);
}

} // namespace cxxlisp
