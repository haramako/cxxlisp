#include <fstream>
#include <iostream>

#include "parser.hpp"
#include "util.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

int main(int argc, char **argv) {
  VM vm;

  for (int i = 1; i < argc; i++) {
    if ("-t"s == argv[i]) {
      vm.EnableTrace = true;
      vm.EnableTraceMacroExpand = true;
      continue;
    }
    ifstream fs(argv[i]);
    if (!fs.is_open()) {
      cerr << "Can't open '" << argv[i] << "'." << endl;
      return 1;
    }
    string src((istreambuf_iterator<char>(fs)), istreambuf_iterator<char>());

    run(vm, src);
    return 0;
  }

  return 0;
}
