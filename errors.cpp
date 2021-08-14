#include "errors.hpp"
#include "value.hpp"
#include "vm.hpp"

namespace cxxlisp {
using namespace std;

string LispException::StackTrace() const {
  stringstream s;
  int i = Stack.size();
  for (auto const &stack : Stack) {
    i--;
    s << i << ": " << stack << endl;
  }
  s << "error: " << what() << endl;
  return s.str();
}

} // namespace cxxlisp
