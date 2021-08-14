#include "vm.hpp"
#include "util.hpp"

namespace cxxlisp {

using namespace std;

//===================================================================
// Env
//===================================================================

bool Env::Get(Atom id, Value &result) const {
  auto it = map_.find(id.Id());
  if (it != map_.end()) {
    result = it->second;
    return true;
  } else if (upper_) {
    return upper_->Get(id, result);
  } else {
    return false;
  }
}

Value Env::GetOr(Atom id, Value default_) const {
  Value r;
  if (Get(id, r)) {
    return r;
  } else {
    return default_;
  }
}

void Env::Define(Atom id, Value v) { map_[id.Id()] = v; }

bool Env::Set(Atom id, Value v) {
  auto it = map_.find(id.Id());
  if (it != map_.end()) {
    map_[id.Id()] = v;
    return true;
  } else if (upper_) {
    return upper_->Set(id, v);
  } else {
    return false;
  }
}

//===================================================================
// Compiler
//===================================================================

Value Compiler::doBegin(Ctx &ctx, Value rest) {
  // cout << "compile: " << car(rest) << endl;
  if (rest.IsNil()) {
    return NIL;
  } else {
    return cons(doValue(ctx, car(rest)), doBegin(ctx, cdr(rest)));
  }
}

Value Compiler::doDefine(Ctx &ctx, Value rest) {
  Value name = car(rest);
  if (name.IsCell()) {
    return list(car(name),
                doValue(ctx, list(ctx.vm->Intern("procedure-set-name!"),
                                  list(SYM_QUOTE, car(name)),
                                  cons(SYM_LAMBDA, cdr(name),
                                       doBegin(ctx, cdr(rest))))));
  } else {
    return list(name, doValue(ctx, car(cdr(rest))));
  }
}

Value Compiler::doSet(Ctx &ctx, Value rest) {
  auto [name, value] = uncons<Atom, Value>(rest);
  return list(name, value);
}

Value Compiler::doIf(Ctx &ctx, Value rest) {
  auto [cond, then, else_] = uncons_rest<Value, Value, Value>(rest);
  if (else_.IsNil()) {
    else_ = list(UNDEF);
  }
  return cons(doValue(ctx, cond), doValue(ctx, then), else_);
}

Value Compiler::doQuote(Ctx &ctx, Value rest) { return car(rest); }

Value Compiler::doLambda(Ctx &ctx, Value rest) {
  return cons(car(rest), doBegin(ctx, cdr(rest)));
}

Value Compiler::doValue(Ctx &ctx, Value code) {
  switch (code.Type()) {
  case ValueType::CELL: {
    return doForm(ctx, code);
  }
  default:
    return code;
  }
}

Value Compiler::doList(Ctx &ctx, Value code) {
  if (code.Type() != ValueType::CELL) {
    if (code.IsNil()) {
      return NIL;
    } else {
      throw LispException("`code` in doList() must be cell");
    }
  } else {
    return cons(doValue(ctx, car(code)), doList(ctx, cdr(code)));
  }
}

Value Compiler::doForm(Ctx &ctx, Value code) {
  // cout << "C:" << code << endl;
  Cell &pair = code.AsCell();
  Value head = pair.Car;
  if (head.IsAtom()) {
    Atom atom = head.AsAtom();
    SpecialForm atom_id = (SpecialForm)atom.Id();
    switch (atom_id) {
    case SpecialForm::BEGIN:
      return cons(head, doBegin(ctx, pair.Cdr));
    case SpecialForm::DEFINE:
      return cons(head, doDefine(ctx, pair.Cdr));
    case SpecialForm::SET_EX:
      return cons(head, doSet(ctx, pair.Cdr));
    case SpecialForm::IF:
      return cons(head, doIf(ctx, pair.Cdr));
    case SpecialForm::LAMBDA:
      return cons(head, doLambda(ctx, pair.Cdr));
    case SpecialForm::QUOTE:
      return code;
    case SpecialForm::LOOP:
      return cons(head, doBegin(ctx, pair.Cdr));
    default: {
      Value f;
      // Macro transform.
      if (ctx.vm->RootEnv().Get(atom, f) && f.IsProcedure()) {
        if (f.AsProcedure().IsMacro()) {
          Value result = Eval().Call(ctx, f, cdr(code));
          return doValue(ctx, result);
        }
      }
    }
    }
  }
  return cons(head, doList(ctx, pair.Cdr));
}

Value Compiler::Compile(VM &vm, Value code) {
  Ctx ctx{&vm, &vm.RootEnv(), NIL};
  Value result;
  try {
    result = doValue(ctx, code);
  } catch (LispException &ex) {
    cout << ex.StackTrace();
    throw;
  }
  return result;
}

//===================================================================
// Eval
//===================================================================

Value Eval::doBegin(Ctx &ctx, Value rest) {
  // cout << "run: " << car(rest) << endl;
  Value r = doValue(ctx, car(rest));
  if (cdr(rest).IsNil()) {
    return r;
  } else {
    return doBegin(ctx, cdr(rest));
  }
}

Value Eval::doDefine(Ctx &ctx, Value rest) {
  auto [name, val] = uncons<Atom, Value>(rest);
  ctx.vm->RootEnv().Define(name, doValue(ctx, val));
  return NIL;
}

Value Eval::doSet(Ctx &ctx, Value rest) {
  auto [name, val] = uncons<Atom, Value>(rest);
  if (ctx.vm->RootEnv().Set(name, doValue(ctx, val))) {
    return NIL;
  } else {
    throw LispException(string("Symbol '") + ctx.vm->AtomToString(name) +
                        "' not found.");
  }
}

Value Eval::doIf(Ctx &ctx, Value rest) {
  auto [cond, then, else_] = uncons_rest<Value, Value, Value>(rest);
  Value v = doValue(ctx, cond);
  if (v.Truthy()) {
    return doValue(ctx, then);
  } else {
    return doBegin(ctx, else_);
  }
}

Value Eval::doQuote(Ctx &ctx, Value rest) { return car(rest); }

Value Eval::doLambda(Ctx &ctx, Value rest) {
  return new Procedure(car(rest), cdr(rest));
}

Value Eval::doLoop(Ctx &ctx, Value rest) {
  try {
    for (;;) {
      doBegin(ctx, rest);
    }
  } catch (BreakException &ex) {
    return ex.Result();
  }
}

Value Eval::doValue(Ctx &ctx, Value code) {
  switch (code.Type()) {
  case ValueType::CELL: {
    return doForm(ctx, code);
  }
  case ValueType::ATOM: {
    Value found;
    if (ctx.env->Get(code.AsAtom(), found)) {
      return found;
    } else {
      stringstream s;
      s << "Symbol " << code << " not found.";
      throw LispException(s.str());
    }
  }
  default:
    return code;
  }
}

Value Eval::doList(Ctx &ctx, Value code) {
  if (code.Type() != ValueType::CELL) {
    if (code.IsNil()) {
      return NIL;
    } else {
      throw "`code` in doList() must be cell";
    }
  } else {
    return new Cell(doValue(ctx, car(code)), doList(ctx, cdr(code)));
  }
}

Value Eval::doForm(Ctx &ctx, Value code) {
  Cell &pair = code.AsCell();
  Value head = pair.Car;
  if (head.IsAtom()) {
    SpecialForm atom_id = (SpecialForm)head.AsAtom().Id();
    switch (atom_id) {
    case SpecialForm::BEGIN:
      return doBegin(ctx, pair.Cdr);
    case SpecialForm::DEFINE:
      return doDefine(ctx, pair.Cdr);
    case SpecialForm::IF:
      return doIf(ctx, pair.Cdr);
    case SpecialForm::LAMBDA:
      return doLambda(ctx, pair.Cdr);
    case SpecialForm::QUOTE:
      return doQuote(ctx, pair.Cdr);
    case SpecialForm::LOOP:
      return doLoop(ctx, pair.Cdr);
    case SpecialForm::SET_EX:
      return doSet(ctx, pair.Cdr);
    default:
      break;
    }
  }

  try {
    return call(ctx, doValue(ctx, head), doList(ctx, pair.Cdr));
  } catch (LispException &ex) {
    ex.Stack.push_back(code.ToString());
    throw;
  }
}

Value Eval::call(Ctx &ctx, Value proc_, Value args) {
  // cout << "call " << proc_ << " " << args << endl;
  auto &proc = proc_.AsProcedure();
  if (proc.IsNative()) {
    // Call native procecure.
    return proc.Func()(ctx, args);
  } else {
    // Call lisp procedure.
    Env *new_env = new Env(ctx.vm, ctx.env);
    Ctx new_ctx{ctx.vm, new_env, proc.Body()};

    // Setup arguments.
    for (Value a = args, p = proc.Params(); !p.IsNil();
         a = cdr(a), p = cdr(p)) {
      if (p.IsCell()) {
        new_env->Define(car(p).AsAtom(), car(a));
      } else {
        new_env->Define(p.AsAtom(), a);
        break;
      }
    }

    Value result;
    try {
      result = doBegin(new_ctx, proc.Body());
    } catch (LispException &ex) {
      ex.Stack.push_back(proc_.ToString());
      throw;
    }
    return result;
  }
}

Value Eval::Call(Ctx &ctx, Value proc, Value args) {
  return call(ctx, proc, args);
}

Value Eval::Execute(Ctx &ctx, Value code) {
  Value result;
  try {
    result = doValue(ctx, code);
  } catch (LispException &ex) {
    cout << ex.StackTrace();
    throw;
  }
  return result;
}

Value Eval::Execute(VM &vm, Value code) {
  Ctx ctx{&vm, &vm.RootEnv(), NIL};
  return Execute(ctx, code);
}

//===================================================================
// VM
//===================================================================

VM::VM() : rootEnv_(this, nullptr) {
  Default = this;

  Intern("begin");
  Intern("define");
  Intern("if");
  Intern("lambda");
  Intern("quote");
  Intern("quasiquote");
  Intern("unquote");
  Intern("loop");
  Intern("set!");
  Intern("unquote-splicing");

  // cout << Intern("quote").Id() << " " << (int)SpecialForm::QUOTE << endl;
}

Atom VM::Intern(const char *v) {
  auto it = atomKeyToId_.find(v);
  if (it != atomKeyToId_.end()) {
    return Atom(it->second);
  } else {
    int new_id = (int)atomKeyToId_.size();
    atomKeyToId_.insert(make_pair(string(v), new_id));
    atomIdToKey_.emplace_back(string(v));
    return Atom(new_id);
  }
}

VM *VM::Default = nullptr;

} // namespace cxxlisp
