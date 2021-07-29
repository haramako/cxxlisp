#include "lexer.hpp"
#include <regex>

namespace cxxlisp {

using namespace std;

const static regex ReNumber(R"(^[0-9]+)");
const static regex ReSymbol(R"(^[a-zA-Z_\-+*/][a-zA-Z_\-+*/.1-9]*)");
const static regex ReString(R"(^"([^"]*)\")");
const static regex ReParens(R"(^[()\[\]{}.#\\'`,;])");
const static regex ReSpaces(R"(^[\s]+)");
const static regex ReLineComment(R"(^;[^\n]*\n)");

Token Lexer::Read(bool skip_comment) {
  if (cur_.has_value()) {
    return *cur_;
  }

  smatch mr;

  // Skip spaces
  for (;;) {

    // Skip and advance position and line.
    auto skip = [](smatch &mr, int &pos, int &line) {
      pos += mr.length();
      for (auto const c : string(mr[0])) {
        if (c == '\n') {
          line++;
        }
      }
    };

    if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReSpaces)) {
      skip(mr, pos_, line_);
    } else if (!skip_comment &&
               regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReLineComment)) {
      skip(mr, pos_, line_);
    } else {
      break;
    }
  }

  if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReNumber)) {
    pos_ += mr.length();
    cur_ = Token((int)stoi(mr[0]));
  } else if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReSymbol)) {
    pos_ += mr.length();
    cur_ = Token(string(mr[0]));
  } else if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReString)) {
    pos_ += mr.length();
    cur_ = Token(TokenType::STRING, string(mr[1]));
  } else if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReParens)) {
    pos_ += mr.length();
    cur_ = Token(string(mr[0])[0]);
  } else {
    cur_ = Token();
  }

  // cout << "token: " << *cur_ << endl;

  return *cur_;
}

void Lexer::Consume() { cur_.reset(); }

} // namespace cxxlisp
