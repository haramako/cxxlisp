#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

static Value string_length(Ctx &ctx, const string &str) {
  return (vint_t)str.size();
}

static Value substring(Ctx &ctx, const string &str, vint_t start, vint_t end) {
  return str.substr(start, end - start);
}

static Value string_append(Ctx &ctx, Value args) {
  stringstream s;
  for (auto v : args) {
    s << v.AsString();
  }
  return s.str();
}

static Value string_to_list(Ctx &ctx, const string &str) {
  Value head = NIL;
  for (int i = (int)str.size() - 1; i >= 0; i--) {
    head = cons((vint_t)str[i], head);
  }
  return head;
}

static Value list_to_string(Ctx &ctx, Value li) {
  string str;
  for (auto c : li) {
    str.push_back((char)c.AsNumber());
  }
  return str;
}

static Value string_to_number(Ctx &ctx, const string &str) { return stoi(str); }
static Value number_to_string(Ctx &ctx, vint_t v) { return to_string(v); }
static Value string_to_symbol(Ctx &ctx, const string &str) {
  return ctx.vm->Intern(str);
}
static Value symbol_to_string(Ctx &ctx, Atom v) {
  return ctx.vm->AtomToString(v);
}

#define F(id, f) add_proc(vm, false, id, f);
#define FV(id, f) add_proc_varg(vm, false, id, f);
#define M(id, f) add_proc(vm, true, id, f);
#define MV(id, f) add_proc_varg(vm, true, id, f);

void lib_string_init(VM &vm) {
  F("string-length", string_length);
  F("substring", substring);
  FV("string-append", string_append);

  F("string->list", string_to_list);
  F("list->string", list_to_string);
  F("string->number", string_to_number);
  F("number->string", number_to_string);
  F("string->symbol", string_to_symbol);
  F("symbol->string", symbol_to_string);
}

} // namespace cxxlisp
