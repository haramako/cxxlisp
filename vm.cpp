#include "vm.hpp"
#include "util.hpp"

namespace cxxlisp {

using namespace std;

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

void Env::Set(Atom id, Value v) { map_[id.Id()] = v; }

//===================================================================
// Eval
//===================================================================

Eval::Eval(VM *vm) : vm_(vm) { env_ = new Env(vm, &vm->RootEnv()); }

Value Eval::doBegin(Value rest) {
  // cout << "run: " << rest << endl;
  Value r = doValue(car(rest));
  if (cdr(rest).IsNil()) {
    return r;
  } else {
    return doBegin(cdr(rest));
  }
}

Value Eval::doDefine(Value rest) {
  auto [name, val] = uncons<Atom, Value>(rest);
  vm_->RootEnv().Set(name, val);
  return NIL;
}

Value Eval::doIf(Value rest) {
  auto [cond, then, else_] = uncons_rest<Value, Value, Value>(rest);
  Value v = doValue(cond);
  if (v.Truthy()) {
    return doValue(then);
  } else {
    return doBegin(else_);
  }
}

Value Eval::doQuote(Value rest) { return car(rest); }

Value Eval::doLambda(Value rest) { return new Procedure(car(rest), cdr(rest)); }

Value Eval::doValue(Value code) {
  switch (code.Type()) {
  case ValueType::CELL: {
    return doForm(code);
  }
  case ValueType::ATOM: {
    Value found;
    if (env_->Get(code.AsAtom(), found)) {
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

Value Eval::doList(Value code) {
  if (code.Type() != ValueType::CELL) {
    if (code.IsNil()) {
      return NIL;
    } else {
      throw "`code` in doList() must be cell";
    }
  } else {
    return new Cell(doValue(car(code)), doList(cdr(code)));
  }
}

Value Eval::doForm(Value code) {
  Cell pair = code.AsCell();
  Value head = pair.Car;
  if (head.IsAtom()) {
    Atom atom = head.AsAtom();
    auto atom_name = vm_->AtomToString(atom);
    if (atom_name == "begin") {
      return doBegin(pair.Cdr);
    } else if (atom_name == "define") {
      return doDefine(pair.Cdr);
    } else if (atom_name == "if") {
      return doIf(pair.Cdr);
    } else if (atom_name == "lambda") {
      return doLambda(pair.Cdr);
    } else {
      // Call procedure.
      return call(doValue(head), doList(pair.Cdr));
    }
  } else if (head.IsSpecial()) {
    if (head == SYM_QUOTE) {
      return doQuote(pair.Cdr);
    } else {
      throw LispException("Unknown special.");
    }
  } else {
    throw LispException("Head of form must be a atom.");
  }
}

Value Eval::call(Value proc_, Value args) {
  auto proc = proc_.AsProcedure();
  return proc.Call(*ctx_, args);
}

Value Eval::Execute(Value code) { return doValue(code); }

//===================================================================
// VM
//===================================================================

VM *VM::Default = nullptr;

} // namespace cxxlisp
