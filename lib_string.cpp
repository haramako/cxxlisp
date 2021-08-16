#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

static Value string_to_list(Ctx &ctx, const string &str) {
  Value head = NIL;
  for (int i = (int)str.size() - 1; i >= 0; i--) {
    head = cons((vint_t)str[i], head);
  }
  return head;
}

static Value string_to_number(Ctx &ctx, const string &str) { return stoi(str); }

static Value string_append(Ctx &ctx, Value args) {
  stringstream s;
  for (auto v : args) {
    s << v.AsString();
  }
  return s.str();
}

static Value string_length(Ctx &ctx, const string &str) {
  return (vint_t)str.size();
}

#define F(id, f) add_proc(vm, false, id, f);
#define FV(id, f) add_proc_varg(vm, false, id, f);
#define M(id, f) add_proc(vm, true, id, f);
#define MV(id, f) add_proc_varg(vm, true, id, f);

void lib_string_init(VM &vm) {
  F("string->list", string_to_list);
  F("string->number", string_to_number);
  F("string-append", string_append);
  F("string-length", string_length);
}

} // namespace cxxlisp
