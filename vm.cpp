#include "vm.hpp"

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
  if (Get(id.Id(), r)) {
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

Value Eval::doValue(Value code) {
  switch (code.Type()) {
  case ValueType::CELL: {
    auto head = doValue(code.AsCell().Car);
    auto rest = doList(code.AsCell().Cdr);
    return call(head, rest);
  }
  case ValueType::ATOM: {
    Value found;
    if (env_->Get(code.AsAtom(), found)) {
      return found;
    } else {
      cout << "*" << code.ToString(*vm_) << endl;
      return code;
      // throw "Not found.";
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
    return new Cell(doValue(code.AsCell().Car), doList(code.AsCell().Cdr));
  }
}

Value Eval::call(Value proc_, Value args) {
  auto proc = proc_.AsProcedure();
  return proc.Call(*vm_, args);
}

Value Eval::Execute(Value code) { return doValue(code); }

//===================================================================
// VM
//===================================================================

VM *VM::Default = nullptr;

} // namespace cxxlisp
