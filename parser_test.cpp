#include <gtest/gtest.h>

#include "parser.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

TEST(ParserTest, Read) {
  VM vm;
  // Parser p{"1"};
  Parser p{"(1 2)"};
  auto v = p.Read(vm);
  cout << v.ToString(vm) << endl;
}
