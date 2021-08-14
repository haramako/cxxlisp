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
  bool result = true;
  if (head.Type() == ValueType::NUMBER) {
    fold<vint_t>(args, [&](vint_t a, vint_t b) {
      result = result && (a < b);
      return b;
    });
    return result;
  } else if (head.Type() == ValueType::STRING) {
    foldc<string>(args, [&](const string &a, const string &b) {
      result = result && (a < b);
      return b;
    });
    return result;
  } else {
    throw BUG();
  }
}

static Value list_(Ctx &ctx, Value args) { return args; }

static Value append_(Ctx &ctx, Value args) {
  Value head = car(args);
  Value tail = car(args);
  for (Value rest = cdr(args); !rest.IsNil(); rest = cdr(rest)) {
    while (!cdr(tail).IsNil()) {
      tail = cdr(tail);
    }
    tail.AsCell().Cdr = car(rest);
  }
  return head;
}

static Value reverse(Ctx &ctx, Value args) {
  Value head = NIL;
  for (Value p = args; !p.IsNil(); p = cdr(p)) {
    head = cons(car(p), head);
  }
  return head;
}

static Value cons_(Ctx &ctx, Value v1, Value v2) { return new Cell(v1, v2); }
static Value car_(Ctx &ctx, Cell &v) { return v.Car; }
static Value cdr_(Ctx &ctx, Cell &v) { return v.Cdr; }
static Value pair_p(Ctx &ctx, Value v) { return v.IsCell(); }
static Value null_p(Ctx &ctx, Value v) { return v.IsNil(); }

static Value break_(Ctx &ctx, Value v) { throw BreakException(v); }

static Value procedure_set_name(Ctx &ctx, Atom name, Procedure &proc) {
  proc.SetName(ctx.vm->AtomToString(name));
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
  for (Value p = args; !p.IsNil(); p = cdr(p)) {
    Value v = car(p);
    cout << v;
    if (!cdr(p).IsNil()) {
      cout << " ";
    }
  }
  cout << endl;
  return NIL;
}

static Value display(Ctx &ctx, Value args) {
  for (auto v : args) {
    if (v.IsString()) {
      cout << v.AsString();
    } else {
      cout << v;
    }
  }
  return NIL;
}

static Value write(Ctx &ctx, Value args) {
  for (auto v : args) {
    cout << v;
  }
  return NIL;
}

static Value qq(Ctx &ctx, Value x, int depth) {
  auto i = [&](const char *a) { return ctx.vm->Intern(a); };

  if (x.IsCell()) {
    Value head = car(x);
    if (head == SYM_UNQUOTE) {
      if (depth <= 0) {
        return car(cdr(x));
      } else {
        return list(i("list"), list(SYM_QUOTE, SYM_UNQUOTE),
                    qq(ctx, car(cdr(x)), depth - 1));
      }
    } else if (head == SYM_UNQUOTE_SPLICING) {
      if (depth <= 0) {
        return list(i("cons"), qq(ctx, head, depth), qq(ctx, cdr(x), depth));
      } else {
        return list(i("list"), list(list(SYM_QUOTE, SYM_UNQUOTE_SPLICING),
                                    qq(ctx, cdr(x), depth - 1)));
      }
    } else if (head == SYM_QUASIQUOTE) {
      return list(i("list"), list(SYM_QUOTE, SYM_QUASIQUOTE),
                  qq(ctx, car(cdr(x)), depth - 1));
    } else if (depth <= 0 && head.IsCell() &&
               car(head) == SYM_UNQUOTE_SPLICING) {
      if (cdr(x).IsNil()) {
        return car(cdr(head));
      } else {
        return list(i("append"), car(cdr(head)), qq(ctx, cdr(x), depth));
      }
    } else {
      return list(i("cons"), qq(ctx, head, 0), qq(ctx, cdr(x), depth));
    }
  } else if (x.IsAtom() || x.IsNil()) {
    return list(SYM_QUOTE, x);
  } else {
    return x;
  }
}

static Value quasiquote(Ctx &ctx, Cell &args) { return qq(ctx, &args, 0); }

#define F(id, f)                                                               \
  {                                                                            \
    auto *proc = make_procedure(f);                                            \
    proc->SetName(id);                                                         \
    env.Define(vm.Intern(id), proc);                                           \
  }
#define FVARG(id, f)                                                           \
  {                                                                            \
    auto *proc = new Procedure(-1, f);                                         \
    proc->SetName(id);                                                         \
    env.Define(vm.Intern(id), proc);                                           \
  }
#define MACRO(id, f)                                                           \
  {                                                                            \
    auto *proc = make_procedure(f);                                            \
    proc->SetIsMacro(true);                                                    \
    env.Define(vm.Intern(id), proc);                                           \
  }
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
  F("car", car_);
  F("cdr", cdr_);
  F("pair?", pair_p);
  F("null?", null_p);

  FVARG("list", list_);
  FVARG("append", append_);
  F("reverse", reverse);

  F("break", break_);

  MACRO_VARG("defmacro", defmacro);
  F("procedure-set-name!", procedure_set_name);
  F("procedure-set-macro!", procedure_set_macro);
  MACRO("quasiquote", quasiquote);

  FVARG("puts", puts);
  FVARG("display", display);
  FVARG("write", write);
}

} // namespace cxxlisp
