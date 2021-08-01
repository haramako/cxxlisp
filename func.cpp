#include "vm.hpp"
#include <functional>

namespace cxxlisp {

using namespace std;

static Value add(VM &vm, vint_t a, vint_t b) { return a + b; }

static Value sub(VM &vm, vint_t a, vint_t b) { return a - b; }

static Value append(VM &vm, const string &a, const string &b) { return a + b; }

#define F(id, f) env.Set(vm.Intern(id), new Procedure(caller(f)));

void init_func(VM &vm) {
  auto &env = vm.RootEnv();
  F("+", add);
  F("-", sub);
  F("append", append);
}

} // namespace cxxlisp
