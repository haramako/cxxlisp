#pragma once
#include <optional>
#include <string>
#include <string_view>

#include <iostream>

namespace cxxlisp {

using namespace std;

enum class TokenType {
  None,
  Number,
  Paren,
  Symbol,
};

class Token {
public:
  TokenType Type;
  int Number;
  char Char;
  std::string Str;

  Token() : Type(TokenType::None), Char('\0') {}
  Token(int v) : Type(TokenType::Number), Number(v) {}
  Token(char v) : Type(TokenType::Paren), Char(v) {}
  Token(const std::string_view v) : Type(TokenType::Symbol), Str(v) {}

  operator std::string() const {
    switch (Type) {
    case TokenType::None:
      return std::string("#EOF");
    case TokenType::Number:
      return std::to_string(Number);
    case TokenType::Paren:
      return std::string{Char};
    case TokenType::Symbol:
      return Str;
    default:
      throw "BUG";
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Token &t) {
    return os << std::string(t);
  }
};

class Lexer {
  std::optional<Token> cur_;

public:
  Lexer(std::string_view s) : s_(s), line_(0), pos_(0){};
  ~Lexer() {}

  Token Read();
  void Consume();
  int Line() const { return line_; }
  int Pos() const { return pos_; }

private:
  std::string s_;
  int line_;
  int pos_;
};

} // namespace cxxlisp
