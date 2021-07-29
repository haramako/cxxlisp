#pragma once
#include <optional>
#include <string>
#include <string_view>

#include <iostream>

namespace cxxlisp {

using namespace std;

enum class TokenType {
  NONE,
  NUMBER,
  PAREN,
  SYMBOL,
  STRING,
};

class Token {
public:
  TokenType Type;
  int Number;
  char Char;
  std::string Str;

  Token() : Type(TokenType::NONE), Char('\0') {}
  Token(int v) : Type(TokenType::NUMBER), Number(v) {}
  Token(char v) : Type(TokenType::PAREN), Char(v) {}
  Token(const std::string_view v) : Type(TokenType::SYMBOL), Str(v) {}
  Token(TokenType tt, const std::string_view v) : Type(tt), Str(v) {}

  operator std::string() const {
    switch (Type) {
    case TokenType::NONE:
      return std::string("#EOF");
    case TokenType::NUMBER:
      return std::to_string(Number);
    case TokenType::PAREN:
      return std::string{Char};
    case TokenType::SYMBOL:
      return Str;
    case TokenType::STRING:
      return '"' + Str + '"';
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

  Token Read(bool skip_comment = false);
  void Consume();
  int Line() const { return line_; }
  int Pos() const { return pos_; }

private:
  std::string s_;
  int line_;
  int pos_;
};

} // namespace cxxlisp
