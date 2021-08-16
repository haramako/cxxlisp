#include <cmath>

#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

inline Value fold_num(Value vals, function<vint_t(vint_t, vint_t)> f) {
  Value head = car(vals);
  if (head.Type() == ValueType::NUMBER) {
    return fold<vint_t>(vals, f);
  } else {
    throw BUG();
  }
}

static Value multiply(Ctx &ctx, Value args) {
  return fold_num(args, [](vint_t a, vint_t b) { return a * b; });
}

static Value divide(Ctx &ctx, Value args) {
  return fold_num(args, [](vint_t a, vint_t b) { return a / b; });
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

static Value modulo(Ctx &ctx, vint_t a, vint_t b) { return a % b; }

static Value abs_(Ctx &ctx, vint_t a) { return abs(a); }
static Value nagative_p(Ctx &ctx, vint_t a) { return a < 0; }
static Value positive_p(Ctx &ctx, vint_t a) { return a >= 0; }
static Value zero_p(Ctx &ctx, vint_t a) { return a == 0; }

static Value min_(Ctx &ctx, Value args) {
  return fold_num(args, [](vint_t a, vint_t b) { return min(a, b); });
}

static Value max_(Ctx &ctx, Value args) {
  return fold_num(args, [](vint_t a, vint_t b) { return max(a, b); });
}

inline Value comp_(Ctx &ctx, Value args, function<bool(vint_t, vint_t)> f_int,
                   function<bool(const string &, const string &)> f_str) {
  Value head = car(args);
  bool result = true;
  if (head.Type() == ValueType::NUMBER) {
    fold<vint_t>(args, [&](vint_t a, vint_t b) {
      result = f_int(a, b);
      return b;
    });
    return result;
  } else if (head.Type() == ValueType::STRING) {
    foldc<string>(args, [&](const string &a, const string &b) {
      result = f_str(a, b);
      return b;
    });
    return result;
  } else {
    throw BUG();
  }
}

static Value greater(Ctx &ctx, Value args) {
  return comp_(
      ctx, args, [](auto a, auto b) { return a > b; },
      [](auto a, auto b) { return a > b; });
}

static Value greater_eq(Ctx &ctx, Value args) {
  return comp_(
      ctx, args, [](auto a, auto b) { return a >= b; },
      [](auto a, auto b) { return a >= b; });
}

static Value less(Ctx &ctx, Value args) {
  return comp_(
      ctx, args, [](auto a, auto b) { return a < b; },
      [](auto a, auto b) { return a < b; });
}

static Value less_eq(Ctx &ctx, Value args) {
  return comp_(
      ctx, args, [](auto a, auto b) { return a <= b; },
      [](auto a, auto b) { return a <= b; });
}

static Value eq_p(Ctx &ctx, Value args) {
  Value cur = car(args);
  for (auto v : cdr(args)) {
    if (cur != v)
      return false;
  }
  return true;
}

#define F(id, f) add_proc(vm, false, id, f);
#define FV(id, f) add_proc_varg(vm, false, id, f);
#define M(id, f) add_proc(vm, true, id, f);
#define MV(id, f) add_proc_varg(vm, true, id, f);

void lib_number_init(VM &vm) {
  FV("+", add);
  FV("-", sub);
  FV("*", multiply);
  FV("/", divide);
  F("modulo", modulo);
  F("abs", abs_);
  F("nagative?", nagative_p);
  F("positive?", positive_p);
  F("zero?", zero_p);
  F("min", min_);
  F("max", max_);
  // F("round", round_);
  // F("floor", floor_);
  // F("ceiling", ceiling_);
  // F("square", square_);

  FV(">", greater);
  FV(">=", greater_eq);
  FV("<", less);
  FV("<=", less_eq);

  FV("eq?", eq_p);
  FV("eqv?", eq_p);
  FV("=", eq_p);
}

} // namespace cxxlisp
