#include <gtest/gtest.h>

#include "parser.hpp"
#include "vm.hpp"

using namespace cxxlisp;
using namespace std;

#if 0
TEST(Lexer, lexer) {
  Lexer lex("hoge");
  auto token = lex.Read();
  EXPECT_EQ("hoge", string(token));
}

TEST(Lexer, ReadString) {
  Lexer lex("\"hoge\"");
  auto token = lex.Read();
  EXPECT_EQ("\"hoge\"", string(token));
}

TEST(Lexer, ReadComment) {
  Lexer lex("; comment \n 1");
  auto token = lex.Read();
  EXPECT_EQ("1", string(token));
}

TEST(ParserTest, Read) {
  VM vm;
  string srcs[] = {"1",     "sym-bol",        "\"string\"", "()",
                   "(1 2)", "(1 a (b c ()))", "#t",         "#f"};
  for (auto const src : srcs) {
    Parser p{&vm, src};
    auto v = p.Read();
    EXPECT_EQ(src, v.ToString(vm));
  }
}
#endif

TEST(ParserTest, ReadWithComment) {
  VM vm;
  string srcs[][2] = {
      {";hoge\n1", "1"},
      {";\n(1\n;hoge\n2)", "(1 2)"},
      {"(1 #;2 3)", "(1 3)"},
      {"(1 #;2)", "(1)"}, // TODO: いまはエラーになってしまう
      {"(1 #;(2) 3)", "(1 3)"},
      {"(1 #;(2))", "(1)"}, // TODO: いまはエラーになってしまう
      {"(#;(2) )", "()"},   // TODO: いまはエラーになってしまう
  };
  for (auto const src : srcs) {
    Parser p{&vm, src[0]};
    auto v = p.Read();
    EXPECT_EQ(src[1], v.ToString(vm));
  }
}

TEST(ParserTest, ReadWithDot) {
  VM vm;
  string srcs[][2] = {
      {"(1 . 2)", "(1 . 2)"},
      {"(1 . (2))", "(1 2)"},
      {"(1 . (2 3))", "(1 2 3)"},
  };
  for (auto const src : srcs) {
    Parser p{&vm, src[0]};
    auto v = p.Read();
    EXPECT_EQ(src[1], v.ToString(vm));
  }
}

TEST(ParserTest, ReadWithReadMacro) {
  VM vm;
  string srcs[][2] = {
      {"'1", "(quote 1)"},
      {"'(1)", "(quote (1))"},
      {"`(1)", "(quasiquote (1))"},
      {",1", "(unquote 1)"},
  };
  for (auto const src : srcs) {
    Parser p{&vm, src[0]};
    auto v = p.Read();
    EXPECT_EQ(src[1], v.ToString(vm));
  }
}
