#include <fstream>

#include "parser.hpp"
#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

Value run(VM &vm, string_view src) {
  Parser parser{vm, src};
  Value result = UNDEF;
  for (;;) {
    Value code;
    try {
      code = parser.Read();
    } catch (EndOfSourceException &) {
      break;
    }

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

Value run_file(VM &vm, string_view path) {
  ifstream fs(string(path).c_str());
  if (!fs.is_open()) {
    throw LispException("Can't open '"s + string(path) + "'.");
  }

  string src((istreambuf_iterator<char>(fs)), istreambuf_iterator<char>());
  return run(vm, src);
}

void add_proc_varg(VM &vm, bool is_macro, const char *id,
                   Value (*f)(Ctx &, Value)) {
  auto *proc = new Procedure(-1, f);
  proc->SetName(id);
  proc->SetIsMacro(is_macro);
  vm.RootEnv().Define(vm.Intern(id), proc);
}

} // namespace cxxlisp
