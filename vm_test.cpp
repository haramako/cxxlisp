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

static Value compile(VM &vm, string_view src) {
  init_func(vm);
  Parser parser{&vm, src};
  Value code = parser.Read();
  // cout << "code " << code << endl;
  Value result = Compiler().Compile(vm, code);
  return result;
}

TEST(CompilerTest, Simple) {
  tuple<const char *, const char *> tests[] = {
      // {expect, test}
      {"1", "1"},
      {"(if 1 2 3)", "(if 1 2 3)"},
      {"(if 1 2 #undef)", "(if 1 2)"},
      {"(define x 1)", "(define x 1)"},
      {"(define f (procedure-set-name f (lambda (x) x)))",
       "(define (f (x) x))"},
  };

  for (const auto &t : tests) {
    VM vm;
    Value result = compile(vm, get<1>(t));
    EXPECT_EQ(string(get<0>(t)), result.ToString(vm));
  }
}

static Value run(VM &vm, string_view src) {
  init_func(vm);
  Parser parser{&vm, src};
  Value code = parser.Read();
  // cout << "code " << code << endl;
  code = Compiler().Compile(vm, code);
  Value result = Eval().Execute(vm, code);
  return result;
}

TEST(EvalTest, Simple) {
  tuple<const char *, const char *> tests[] = {
      // {expect, test}
      {"1", "(+ 1)"},
      {"3", "(+ 1 2)"},
      {"6", "(+ 1 2 3)"},
      {"\"a\"", R"((+ "a"))"},
      {"\"ab\"", R"((+ "a" "b"))"},
      {"\"abc\"", R"((+ "a" "b" "c"))"},
      {"(1 . 2)", R"((cons 1 2))"},
      {"(1 2)", R"((list 1 2))"},
      {"1", R"((begin (define x 1) x))"},
      {"()", R"('())"},
      {"1", R"((if #t 1 2))"},
      {"2", R"((if #f 1 2))"},
      {"2", R"((if '() 1 2))"},
      {"1", R"(((lambda (x) x) 1))"},
      {"(1 2)", R"(((lambda x x) 1 2))"},
      {"(1 (2 3))", R"(((lambda (x . y) (list x y)) 1 2 3))"},
      {"1", R"((begin (define x 1) ((lambda () x))))"},
      //{"1", R"((begin (define x 1) ((lambda () y))))"},
  };

  for (const auto &t : tests) {
    VM vm;
    Value result = run(vm, get<1>(t));
    EXPECT_EQ(get<0>(t), result.ToString());
  }
}
