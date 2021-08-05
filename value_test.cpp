#include <gtest/gtest.h>
#include <string>

#include "util.hpp"
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

static Value func0(Ctx &ctx) { return 0; }
static Value func1(Ctx &ctx, vint_t arg1) { return arg1; }

TEST(ValueTest, Procedure) {
  VM vm;
  Ctx ctx{&vm, &vm.RootEnv()};

  auto proc0 = make_procedure(func0);
  auto proc1 = make_procedure(func1);

  EXPECT_EQ(0, proc0->Arity());
  EXPECT_EQ(0, proc0->Call(ctx, NIL));

  EXPECT_EQ(1, proc1->Arity());
  EXPECT_EQ(1, proc1->Call(ctx, cons(1, NIL)));
}
