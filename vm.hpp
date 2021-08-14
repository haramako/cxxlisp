#pragma once
#include <unordered_map>
#include <vector>

#include "value.hpp"

namespace cxxlisp {

class Env;
class VM;

struct Ctx {
  VM *vm;
  Env *env;
  Value code;
  Ctx(VM *v, Env *e, Value c) : vm(v), env(e), code(c) {}
};

class Env : public gc_cleanup {
  VM *vm_;
  Env *upper_;
  std::unordered_map<atom_id_t, Value> map_;

public:
  Env(VM *vm, Env *upper) : vm_(vm), upper_(upper) {}
  bool Get(Atom id, Value &result) const;
  Value GetOr(Atom id, Value default_ = NIL) const;
  void Define(Atom id, Value v);
  bool Set(Atom id, Value v);

  int Count() { return map_.size(); }
};

/**
 * Compiler.
 *
 * Macro processor.
 */
class Compiler {
  Value doBegin(Ctx &ctx, Value rest);
  Value doDefine(Ctx &ctx, Value rest);
  Value doSet(Ctx &ctx, Value rest);
  Value doIf(Ctx &ctx, Value rest);
  Value doQuote(Ctx &ctx, Value rest);
  Value doLambda(Ctx &ctx, Value rest);

  Value doValue(Ctx &ctx, Value code);
  Value doList(Ctx &ctx, Value code);
  Value doForm(Ctx &ctx, Value code);

public:
  Compiler() {}
  Value Compile(VM &vm, Value code);
};

class Eval {
  Value doBegin(Ctx &ctx, Value rest);
  Value doDefine(Ctx &ctx, Value rest);
  Value doSet(Ctx &ctx, Value rest);
  Value doIf(Ctx &ctx, Value rest);
  Value doQuote(Ctx &ctx, Value rest);
  Value doLambda(Ctx &ctx, Value rest);
  Value doLoop(Ctx &ctx, Value rest);

  Value doValue(Ctx &ctx, Value code);
  Value doList(Ctx &ctx, Value code);
  Value doForm(Ctx &ctx, Value code);
  Value call(Ctx &ctx, Value proc, Value args);

public:
  Eval(){};

  Value Execute(VM &vm, Value code);
  Value Execute(Ctx &ctx, Value code);

  Value Call(Ctx &ctx, Value proc, Value args);
};

/**
 * List Virtual Machine.
 */
class VM {
  std::unordered_map<std::string, Atom> atomKeyToId_;
  std::vector<std::string> atomIdToKey_;
  Env rootEnv_;

  void init();

public:
  bool EnableStackTrace = true;
  bool EnableTrace = false;
  bool EnableTraceMacroExpand = false;

  static VM *Default;

  VM();
  VM(const VM &) = delete;
  VM &operator=(const VM &) = delete;

  Atom Intern(const char *v);
  Atom Intern(const std::string &v) { return Intern(v.c_str()); }
  const std::string &AtomToString(Atom atom) const {
    return atomIdToKey_[atom.Id()];
  }

  Env &RootEnv() { return rootEnv_; }
};

/**
 * 'break' in loop.
 */
class BreakException : public std::exception {
  Value result_;

public:
  BreakException(Value v) : result_(v) {}

  Value Result() const { return result_; }
};

// From func.cpp
void init_func(VM &vm);

} // namespace cxxlisp
