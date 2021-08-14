#include "util.hpp"
#include "parser.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

Value run(VM &vm, string_view src) {
  init_func(vm);
  Parser parser{&vm, src};
  Value result = NIL;
  for (;;) {
    Value code = parser.Read();
    if (code.IsNil())
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

} // namespace cxxlisp