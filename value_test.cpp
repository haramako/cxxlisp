#include <gtest/gtest.h>
#include <string>

#include "value.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

TEST(ValueTest, Copyable) {
  Value v1{1};
  Value v2{v1};
  EXPECT_EQ(v1, v2);
}

TEST(ValueTest, Equal) {
  EXPECT_NE(Value(1), NIL);
  EXPECT_NE(Value(1), Value("hoge"));
}

TEST(ValueTest, Nil) {
  EXPECT_EQ(NIL, NIL);
  EXPECT_TRUE(NIL.IsNil());
}

TEST(ValueTest, Number) {
  EXPECT_EQ(Value(1), Value(1));
  EXPECT_NE(Value(2), Value(1));
  EXPECT_EQ(1, Value(1).AsNumber());
}

TEST(ValueTest, Intern) {
  VM vm;
  EXPECT_EQ(vm.Intern("hoge"), vm.Intern("hoge"));
  EXPECT_NE(vm.Intern("fuga"), vm.Intern("hoge"));
  EXPECT_EQ("hoge", vm.AtomToString(vm.Intern("hoge")));
}

TEST(ValueTest, String) {
  EXPECT_EQ(Value("hoge"), Value("hoge"));
  EXPECT_NE(Value("hoge"), Value("fuga"));
  EXPECT_EQ("hoge", Value("hoge").AsString());
}

TEST(ValueTest, Hoge) {
  EXPECT_EQ(true, true);
  1;
}
