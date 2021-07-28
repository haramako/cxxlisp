#include <iostream>

#include "parser.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

Value Parser::Read(VM &vm) {
  stack_.clear();
  Token t = lex_.Read();
  switch (t.Type) {
  case TokenType::None:
    lex_.Consume();
    return stack_[0];

  case TokenType::Number:
    lex_.Consume();
    return Value(t.Number);

  case TokenType::Paren: {
    lex_.Consume();
    switch (t.Char) {
    case '(': {
      Cell *head = nullptr;
      Cell *tail = nullptr;
      for (;;) {
        auto t2 = lex_.Read();
        if (t2.Type == TokenType::Paren && t2.Char == ')') {
          if (!head) {
            return NIL;
          } else {
            return head;
          }
        }

        if (!head) {
          head = new Cell();
          tail = head;
        } else {
          Cell *new_tail = new Cell();
          tail->Cdr = new_tail;
          tail = new_tail;
        }
        tail->Car = Read(vm);
      }
    }
    case ')':
      throw "BUG";
    }
  }

  case TokenType::Symbol:
    lex_.Consume();
    return Value(vm.Intern(t.Str));
  }
  return NIL;
}

} // namespace cxxlisp
