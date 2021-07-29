#pragma once

#include <string>

#include "lexer.hpp"
#include "value.hpp"

namespace cxxlisp {

class VM;

class Parser {
  VM *vm_;
  Lexer lex_;

  Value parseList();
  Value parseReadMacro();

public:
  Parser(VM *vm, std::string_view s) : vm_(vm), lex_(s) {}
  Value Read();
};

} // namespace cxxlisp
