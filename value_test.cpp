#include <gtest/gtest.h>
#include <string>

#include "value.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

TEST(Value, Copyable) {
  Value v{1};
  Value v2{v}; // Must not compile
  EXPECT_EQ(v, v2);
}

TEST(Value, Equal) {
  EXPECT_NE(Value(1), NIL);
  EXPECT_NE(Value(1), Value("hoge"));
}

TEST(Value, Nil) {
  EXPECT_EQ(NIL, NIL);
  EXPECT_TRUE(NIL.IsNil());
}

TEST(Value, Number) {
  EXPECT_EQ(Value(1), Value(1));
  EXPECT_NE(Value(2), Value(1));
  EXPECT_EQ(1, Value(1).AsNumber());
}

TEST(Value, Intern) {
  VM vm;
  EXPECT_EQ(vm.Intern("hoge"), vm.Intern("hoge"));
  EXPECT_NE(vm.Intern("fuga"), vm.Intern("hoge"));
  EXPECT_EQ("hoge", vm.AtomToString(vm.Intern("hoge")));
}

TEST(Value, String) {
  EXPECT_EQ(Value("hoge"), Value("hoge"));
  EXPECT_NE(Value("hoge"), Value("fuga"));
  EXPECT_EQ("hoge", Value("hoge").AsString());
}
