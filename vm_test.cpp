#include <gtest/gtest.h>
#include <string>

#include "parser.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

TEST(EnvTest, GetAndSet) {
  VM vm;
  Env env{&vm, nullptr};
  env.Set(vm.Intern("hoge"), Value(1));
  env.Set(vm.Intern("fuga"), Value(2));
  EXPECT_EQ(1, env.GetOr(vm.Intern("hoge")));
  EXPECT_EQ(2, env.GetOr(vm.Intern("fuga")));
  EXPECT_EQ(NIL, env.GetOr(vm.Intern("piyo")));
}

TEST(EnvTest, GetFromUpper) {
  VM vm;
  Env &root = vm.RootEnv();
  Env env{&vm, &root};
  vm.RootEnv().Set(vm.Intern("upper"), 1);
  EXPECT_EQ(1, root.GetOr(vm.Intern("upper")));
  EXPECT_EQ(1, env.GetOr(vm.Intern("upper")));
}

Value run(VM &vm, string_view src) {
  init_func(vm);
  Eval eval{&vm};
  Parser parser{&vm, src};
  Value code = parser.Read();
  Value result = eval.Execute(code);
  return result;
}

TEST(EvalTest, Simple) {
  VM vm;
  tuple<const char *, const char *> tests[] = {
      {"1", "(+ 1)"}, // {expect, test}
      {"3", "(+ 1 2)"},
      {"6", "(+ 1 2 3)"},
      {"\"a\"", R"((+ "a"))"},
      {"\"ab\"", R"((+ "a" "b"))"},
      {"\"abc\"", R"((+ "a" "b" "c"))"},
      {"(1 . 2)", R"((cons 1 2))"},
      {"(1 2)", R"((list 1 2))"},
  };

  for (const auto &t : tests) {
    Value result = run(vm, get<1>(t));
    EXPECT_EQ(get<0>(t), result.ToString());
  }
}
