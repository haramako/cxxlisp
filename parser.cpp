#include "parser.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

Value Cons(Value car, Value cdr) { return new Cell(car, cdr); }

Value Parser::Read() {
  Token t = lex_.Read();
  switch (t.Type) {
  case TokenType::NONE:
    lex_.Consume();
    return NIL;

  case TokenType::NUMBER:
    lex_.Consume();
    return Value(t.Number);

  case TokenType::PAREN: {
    lex_.Consume();
    switch (t.Char) {
    case '(':
      return parseList();
    case '#':
      return parseReadMacro();
    case '\'':
      return Cons(SYM_QUOTE, Cons(Read(), NIL));
    case '`':
      return Cons(SYM_QUASIQUOTE, Cons(Read(), NIL));
    case ',':
      return Cons(SYM_UNQUOTE, Cons(Read(), NIL));
    case ')':
    case '.':
    default:
      throw "BUG";
    }
  }

  case TokenType::SYMBOL:
    lex_.Consume();
    return Value(vm_->Intern(t.Str));
  case TokenType::STRING:
    lex_.Consume();
    return Value(t.Str);
  }
  return NIL;
}

Value Parser::parseList() {
  Cell *head = nullptr;
  Cell *tail = nullptr;
  for (;;) {
    auto t = lex_.Read();
    if (t.Type == TokenType::PAREN && t.Char == ')') {
      lex_.Consume();
      // End of list (a b | )
      if (!head) {
        return NIL;
      } else {
        return head;
      }
    } else if (t.Type == TokenType::PAREN && t.Char == '.') {
      // Dot list (a | . b)
      lex_.Consume();
      if (!head) {
        throw "BUG";
      } else {
        tail->Cdr = Read();
        return head;
      }
    } else {
      // Normal list element (a | b ...)
      if (!head) {
        head = new Cell();
        tail = head;
      } else {
        Cell *new_tail = new Cell();
        tail->Cdr = new_tail;
        tail = new_tail;
      }
      tail->Car = Read();
    }
  }
}

Value Parser::parseReadMacro() {
  auto t = lex_.Read(true);
  if (t.Type == TokenType::SYMBOL) {
    lex_.Consume();
    if (t.Str == "f") {
      // #f
      return BOOL_F;
    } else if (t.Str == "t") {
      // #t
      return BOOL_T;
    } else {
      throw "BUG";
    }
  } else if (t.Type == TokenType::PAREN) {
    lex_.Consume();
    if (t.Char == ';') {
      // SEXP comment.
      Read(); // discard SEXP as comment.
      return Read();
    } else {
      throw "BUG";
    }
  } else {
    throw "BUG";
  }
}

} // namespace cxxlisp
