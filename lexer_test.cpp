#include <gtest/gtest.h>

#include "lexer.hpp"

using namespace cxxlisp;
using namespace std;

TEST(Lexer, lexer) {
  Lexer lex("hoge");
  auto token = lex.Read();
  EXPECT_EQ("hoge", string(token));
}
