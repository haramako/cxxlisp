#pragma once

#include <iostream>
#include <optional>
#include <string>

#include "value.hpp"

namespace cxxlisp {

class VM;

enum class TokenType {
  EOS, // End of stream.
  NUMBER,
  SYMBOL,
  IDENT,
  STRING,
};

class Token {
public:
  TokenType Type;
  int Number;
  char Char;
  std::string Str;

  Token() : Type(TokenType::EOS), Char('\0') {}
  Token(int v) : Type(TokenType::NUMBER), Number(v) {}
  Token(char v) : Type(TokenType::SYMBOL), Char(v) {}
  Token(const std::string_view v) : Type(TokenType::IDENT), Str(v) {}
  Token(TokenType tt, const std::string_view v) : Type(tt), Str(v) {}

  operator std::string() const;

  friend std::ostream &operator<<(std::ostream &os, const Token &t) {
    return os << std::string(t);
  }
};

class Parser {
  VM *vm_;

  std::string s_;
  std::optional<Token> cur_;
  int line_ = 0;
  int pos_ = 0;

  Token next();
  void consume();

  Value parseList();
  Value parseReadMacro();

public:
  int Line() const { return line_; }
  int Pos() const { return pos_; }

  Parser(VM *vm, std::string_view s) : vm_(vm), s_(s) {}
  Value Read();
};

} // namespace cxxlisp
