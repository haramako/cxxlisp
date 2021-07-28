#pragma once

#include <string>
#include <vector>

#include "lexer.hpp"
#include "value.hpp"

namespace cxxlisp {

class VM;

class Parser {
  Lexer lex_;
  std::vector<Value> stack_;

public:
  Parser(std::string_view s) : lex_(s) {}
  Value Read(VM &vm);
  void Consume();
};

} // namespace cxxlisp
