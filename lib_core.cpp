#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

static Value null_p(Ctx &ctx, Value v) { return v.IsNil(); }
static Value number_p(Ctx &ctx, Value v) { return v.IsNumber(); }
static Value pair_p(Ctx &ctx, Value v) { return v.IsCell(); }
static Value string_p(Ctx &ctx, Value v) { return v.IsString(); }
static Value boolean_p(Ctx &ctx, Value v) { return v.IsBoolean(); }
static Value procedure_p(Ctx &ctx, Value v) { return v.IsProcedure(); }
static Value symbol_p(Ctx &ctx, Value v) { return v.IsAtom(); }

static Value not_(Ctx &ctx, bool v) { return !v; }
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

static Value macroexpand(Ctx &ctx, Value code) {
  return Compiler().Expand(ctx, code);
}

static Value macroexpand_1(Ctx &ctx, Value code) {
  return Compiler().ExpandOne(ctx, code);
}

static Value load(Ctx &ctx, const string &filename) {
  return run_file(*ctx.vm, filename);
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

#define F(id, f) add_proc(vm, false, id, f);
#define FV(id, f) add_proc_varg(vm, false, id, f);
#define M(id, f) add_proc(vm, true, id, f);
#define MV(id, f) add_proc_varg(vm, true, id, f);

void lib_core_init(VM &vm) {
  F("null?", null_p);
  F("number?", number_p);
  F("pair?", pair_p);
  F("strng?", string_p);
  F("boolean?", boolean_p);
  F("procedure?", procedure_p);
  F("symbol?", symbol_p);

  F("not", not_);
  F("break", break_);

  MV("defmacro", defmacro);
  F("procedure-set-name!", procedure_set_name);
  F("procedure-set-macro!", procedure_set_macro);
  M("quasiquote", quasiquote);
  F("macroexpand", macroexpand);
  F("macroexpand-1", macroexpand_1);

  FV("puts", puts);
  FV("display", display);
  FV("write", write);

  F("load", load);
}

} // namespace cxxlisp
