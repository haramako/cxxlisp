#include "lexer.hpp"
#include <regex>

namespace cxxlisp {
using namespace std;

const static regex ReNumber(R"(^[0-9]+)");
const static regex ReSymbol(R"(^[a-zA-Z_\-.][a-zA-Z1-9_\-.]*)");
const static regex ReParens(R"(^[()\[\]{}])");
const static regex ReSpaces(R"(^[\s]+)");

Token Lexer::Read() {
  if (cur_.has_value()) {
    return *cur_;
  }

  smatch mr;

  // Skip spaces
  if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReSpaces)) {
    pos_ += mr.length();
    for (auto const c : string(mr[0])) {
      if (c == '\n') {
        line_++;
      }
    }
  }

  if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReNumber)) {
    pos_ += mr.length();
    cur_ = Token((int)stoi(string(mr[0])));
  } else if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReSymbol)) {
    pos_ += mr.length();
    cur_ = Token(string(mr[0]));
  } else if (regex_search(s_.cbegin() + pos_, s_.cend(), mr, ReParens)) {
    pos_ += mr.length();
    cur_ = Token(string(mr[0])[0]);
  } else {
    cur_ = Token();
  }

  return *cur_;
}

void Lexer::Consume() { cur_.reset(); }

} // namespace cxxlisp
