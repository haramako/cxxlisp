#pragma once
#include <unordered_map>
#include <vector>

#include "value.hpp"

namespace cxxlisp {

class Env {
  VM *vm_;
  Env *upper_;
  std::unordered_map<atom_id_t, Value> map_;

public:
  Env(VM *vm, Env *upper) : vm_(vm), upper_(upper) {}
  bool Get(Atom id, Value &result) const;
  Value GetOr(Atom id, Value default_ = NIL) const;
  void Set(Atom id, Value v);
};

class Eval {
  VM *vm_;
  Env *env_;

  Value doBegin(Value rest);
  Value doDefine(Value rest);

  Value doValue(Value code);
  Value doList(Value code);
  Value doForm(Value code);
  Value call(Value proc, Value args);

public:
  Eval(VM *vm);

  Value Execute(Value code);
};

class Compiler {
  VM *vm_;

public:
  Compiler(VM *vm) : vm_(vm) {}
};

/**
 * List Virtual Machine.
 */
class VM {
  std::unordered_map<std::string, Atom> atomKeyToId_;
  std::vector<std::string> atomIdToKey_;
  Env rootEnv_;

public:
  static VM *Default;

  VM() : rootEnv_(this, nullptr) { Default = this; }
  VM(const VM &) = delete;
  VM &operator=(const VM &) = delete;

  Atom Intern(const char *v);
  Atom Intern(const std::string &v) { return Intern(v.c_str()); }
  const std::string &AtomToString(Atom atom) const {
    return atomIdToKey_[atom.Id()];
  }

  Env &RootEnv() { return rootEnv_; }
};

// From func.cpp
void init_func(VM &vm);

} // namespace cxxlisp
