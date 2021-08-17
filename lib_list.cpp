#include <cmath>

#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

static Value cons_(Ctx &ctx, Value v1, Value v2) { return new Cell(v1, v2); }
static Value car_(Ctx &ctx, Cell &v) { return v.Car; }
static Value cdr_(Ctx &ctx, Cell &v) { return v.Cdr; }
static Value set_car_i(Ctx &ctx, Cell &c, Value v) { return c.Car = v; }
static Value set_cdr_i(Ctx &ctx, Cell &c, Value v) { return c.Cdr = v; }

static Value list_(Ctx &ctx, Value args) { return args; }

static Value append_(Ctx &ctx, Value args) {
  Value head = car(args);
  Value tail = head;

  if (head.IsNil()) {
    return append_(ctx, cdr(args));
  }

  for (Value rest = cdr(args); !rest.IsNil(); rest = cdr(rest)) {
    while (!cdr(tail).IsNil()) {
      tail = cdr(tail);
    }
    if (!car(rest).IsNil()) {
      tail.AsCell().Cdr = car(rest);
    }
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

#define F(id, f) add_proc(vm, false, id, f);
#define FV(id, f) add_proc_varg(vm, false, id, f);
#define M(id, f) add_proc(vm, true, id, f);
#define MV(id, f) add_proc_varg(vm, true, id, f);

void lib_list_init(VM &vm) {
  F("cons", cons_);
  F("car", car_);
  F("cdr", cdr_);
  F("set-car!", set_car_i);
  F("set-cdr!", set_cdr_i);

  FV("list", list_);
  FV("append", append_);
  F("reverse", reverse);
}

} // namespace cxxlisp
