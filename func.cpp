#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

template <typename T> T fold(Value list, function<T(T, T)> f) {
  Value head = car(list);
  Value rest = cdr(list);
  T r = val_as<T>(head);
  for (auto v : rest) {
    r = f(r, val_as<T>(v));
  }
  return r;
}

template <typename T> T foldc(Value list, function<T(const T &, const T &)> f) {
  Value head = car(list);
  Value rest = cdr(list);
  T r = val_as<const T &>(head);
  for (auto v : rest) {
    r = f(r, val_as<const T &>(v));
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

static Value greater(Ctx &ctx, Value args) {
  Value head = car(args);
  bool result = true;
  if (head.Type() == ValueType::NUMBER) {
    fold<vint_t>(args, [&](vint_t a, vint_t b) {
      result = result && (a > b);
      return b;
    });
    return result;
  } else if (head.Type() == ValueType::STRING) {
    foldc<string>(args, [&](const string &a, const string &b) {
      result = result && (a > b);
      return b;
    });
    return result;
  } else {
    throw BUG();
  }
}

static Value less(Ctx &ctx, Value args) {
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

static Value list_(Ctx &ctx, Value args) { return args; }

static Value cons_(Ctx &ctx, Value v1, Value v2) { return new Cell(v1, v2); }

static Value break_(Ctx &ctx, Value v) { throw BreakException(v); }

static Value procedure_set_name(Ctx &ctx, const string &name, Procedure &proc) {
  proc.SetName(name);
  return &proc;
}

static Value procedure_set_macro(Ctx &ctx, Procedure &proc) {
  proc.SetIsMacro(true);
  return &proc;
}

static Value defmacro(Ctx &ctx, Value args) {
  auto [name, arg_list, code] = uncons_rest<Atom, Value, Value>(args);
  auto i = [&](const char *a) { return ctx.vm->Intern(a); };

  return list(
      i("define"), name,
      list(i("procedure-set-macro!"), cons(i("lambda"), arg_list, code)));
}

static Value puts(Ctx &ctx, Value args) {
  for (auto v : args) {
    cout << v << " ";
  }
  cout << endl;
  return NIL;
}

static Value display(Ctx &ctx, Value args) {
  for (auto v : args) {
    if (v.IsString()) {
      cout << v.AsString() << " ";
    } else {
      cout << v << " ";
    }
  }
  return NIL;
}

static Value write(Ctx &ctx, Value args) {
  for (auto v : args) {
    cout << v << " ";
  }
  return NIL;
}

#define F(id, f) env.Define(vm.Intern(id), make_procedure(f));
#define FVARG(id, f) env.Define(vm.Intern(id), new Procedure(-1, f));
#define MACRO_VARG(id, f)                                                      \
  {                                                                            \
    auto *proc = new Procedure(-1, f);                                         \
    proc->SetIsMacro(true);                                                    \
    env.Define(vm.Intern(id), proc);                                           \
  }

void init_func(VM &vm) {
  auto &env = vm.RootEnv();
  FVARG("+", add);
  FVARG("-", sub);
  FVARG(">", greater);
  FVARG("<", less);
  F("cons", cons_);
  FVARG("list", list_);

  F("break", break_);

  MACRO_VARG("defmacro", defmacro);
  F("procedure-set-name!", procedure_set_name);
  F("procedure-set-macro!", procedure_set_macro);

  FVARG("puts", puts);
  FVARG("display", display);
  FVARG("write", write);
}

} // namespace cxxlisp
