#include <fstream>
#include <iostream>

#include "parser.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

static Value run(VM &vm, string_view src) {
  init_func(vm);
  Eval eval{&vm};
  Parser parser{&vm, src};
  Value result;
  for (;;) {
    Value code = parser.Read();
    if (code.IsNil()) {
      break;
    }
    result = eval.Execute(code);
  }
  return result;
}

int main(int argc, char **argv) {
  VM vm;

  for (int i = 1; i < argc; i++) {
    ifstream fs(argv[i]);
    if (!fs.is_open()) {
      cerr << "Can't open '" << argv[i] << "'." << endl;
      return 1;
    }
    string src((istreambuf_iterator<char>(fs)), istreambuf_iterator<char>());
    Value result = run(vm, src.c_str());
    cout << result << endl;
    return 0;
  }

  return 0;
}
