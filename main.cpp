#include <iostream>

#include "lexer.hpp"

using namespace cxxlisp;
using namespace std;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  Lexer lex(R"(
(define (hoge fuga123)
  sym-bol sym_bol 1 012
  . .. () (hoge))
)");
  for (auto s = lex.Read(); s.Type != TokenType::NONE; s = lex.Read()) {
    std::cout << s << endl;
    // std::cout << lex.Line() << " " << s << endl;
  }
  return 0;
}
