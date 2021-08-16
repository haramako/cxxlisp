#include "util.hpp"
#include "parser.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

Value run(VM &vm, string_view src) {
  init_func(vm);
  Parser parser{vm, src};
  Value result = UNDEF;
  for (;;) {
    Value code = parser.Read();
    if (code == END_OF_SOURCE)
      break;
    if (vm.EnableTrace) {
      cout << "trace: " << code << endl;
    }
    code = Compiler().Compile(vm, code);
    if (vm.EnableTraceMacroExpand) {
      cout << "trace: expand ";
      pretty_print(cout, vm, code, 1000);
      cout << endl;
    }
    result = Eval().Execute(vm, code);
  }
  return result;
}

void add_proc_varg(VM &vm, bool is_macro, const char *id,
                   Value (*f)(Ctx &, Value)) {
  auto *proc = new Procedure(-1, f);
  proc->SetName(id);
  proc->SetIsMacro(is_macro);
  vm.RootEnv().Define(vm.Intern(id), proc);
}

} // namespace cxxlisp
