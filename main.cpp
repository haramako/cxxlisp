#include <fstream>
#include <iostream>

#include "parser.hpp"
#include "util.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

int main(int argc, char **argv) {
  VM vm;
  init_func(vm);

  for (int i = 1; i < argc; i++) {
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
