#include <regex>

#include "parser.hpp"
#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

const static regex RE_NUMBER(R"(^[0-9]+)");
const static regex RE_IDENT(R"(^[a-zA-Z_\-+*/<>=!?][a-zA-Z_\-+*/<>=!?.1-9]*)");
const static regex RE_STRING(R"(^"([^"]*)\")");
const static regex RE_SYMBOL(R"(^[()\[\]{}.#\\'`,@;])");
const static regex RE_SPACES(R"(^[\s]+)");
const static regex RE_LINE_COMMENT(R"(^;[^\n]*\n)");
const static regex RE_SEXP_COMMENT(R"(^#;)");

string unescape_str(string_view str) {
  if (str.find('\\') == string::npos) {
    return string(str);
  }

  string buf;
  buf.reserve(str.length());

  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '\\' && i < str.length() - 1) {
      switch (str[i + 1]) {
      case 'n':
        buf.push_back('\n');
        i++;
        continue;
      case '\\':
        buf.push_back('\\');
        i++;
        continue;
      default:
        break;
      }
    } else {
      buf.push_back(str[i]);
    }
  }
  return buf;
}

string escape_str(string_view str) {
  string buf;
  buf.reserve(str.length());

  for (size_t i = 0; i < str.length(); i++) {
    switch (str[i]) {
    case '\n':
      buf.append("\\n");
      continue;
    case '\t':
      buf.append("\\t");
      continue;
    default:
      buf.push_back(str[i]);
      break;
    }
  }
  return buf;
}

Token::operator string() const {
  switch (Type) {
  case TokenType::EOS:
    return std::string("#EOS");
  case TokenType::NUMBER:
    return std::to_string(Number);
  case TokenType::SYMBOL:
    return std::string{Char};
  case TokenType::IDENT:
    return Str;
  case TokenType::STRING:
    return '"' + escape_str(Str) + '"';
  default:
    throw BUG();
  }
}

//==============================================================================
// Parser
//==============================================================================

Token Parser::next() {

  if (unreaded_) {
    unreaded_ = false;
    return cur_;
  }

  smatch mr;

  auto search = [&](smatch &result, const regex &re) {
    if (regex_search(s_.cbegin() + pos_, s_.cend(), result, re)) {
      pos_ += mr.length();
      return true;
    } else {
      return false;
    }
  };

  // Skip spaces.
  for (;;) {

    // Skip and advance position and line.
    auto skip = [](smatch &mr, int &pos, int &line) {
      auto s = mr.str();
      line += count_if(s.begin(), s.end(), [](char c) { return c == '\n'; });
    };

    if (search(mr, RE_SPACES)) {
      skip(mr, pos_, line_);
    } else if (search(mr, RE_LINE_COMMENT)) {
      skip(mr, pos_, line_);
    } else if (search(mr, RE_SEXP_COMMENT)) {
      skip(mr, pos_, line_);
      Read(); // Discard SEXP.
    } else {
      break;
    }
  }

  if (search(mr, RE_NUMBER)) {
    cur_ = Token((int)stoi(mr[0]));
  } else if (search(mr, RE_IDENT)) {
    cur_ = Token(string(mr[0]));
  } else if (search(mr, RE_STRING)) {
    cur_ = Token(TokenType::STRING, unescape_str(string(mr[1])));
  } else if (search(mr, RE_SYMBOL)) {
    cur_ = Token(string(mr[0])[0]);
  } else {
    cur_ = Token();
  }

  // cout << "token: " << *cur_ << endl;

  return cur_;
}

void Parser::unread() {
  if (unreaded_) {
    throw LispException("Can't unread token.");
  } else {
    unreaded_ = true;
  }
}

Value Parser::Read() {
  Token t = next();
  switch (t.Type) {
  case TokenType::EOS:
    throw EndOfSourceException();

  case TokenType::NUMBER:
    return Value(t.Number);

  case TokenType::SYMBOL: {
    switch (t.Char) {
    case '(':
      return parseList();
    case '#':
      return parseReadMacro();
    case '\'':
      return list(SYM_QUOTE, Read());
    case '`':
      return list(SYM_QUASIQUOTE, Read());
    case ',': {
      t = next();
      if (t.Type == TokenType::SYMBOL && t.Char == '@') {
        return list(SYM_UNQUOTE_SPLICING, Read());
      } else {
        unread();
        return list(SYM_UNQUOTE, Read());
      }
    }
    case ')':
    case '.':
    default:
      throw BUG();
    }
  }

  case TokenType::IDENT:
    return Value(vm_.Intern(t.Str));
  case TokenType::STRING:
    return Value(t.Str);
  default:
    throw BUG();
  }
}

Value Parser::parseList() {
  Cell *head = nullptr;
  Cell *tail = nullptr;
  for (;;) {
    auto t = next();
    if (t.Type == TokenType::SYMBOL && t.Char == ')') {
      // End of list (a b | )
      if (!head) {
        return NIL;
      } else {
        return head;
      }
    } else if (t.Type == TokenType::SYMBOL && t.Char == '.') {
      // Dot list (a | . b)
      if (!head) {
        throw BUG();
      } else {
        tail->Cdr = Read();
        t = next();
        if (t.Type == TokenType::SYMBOL && t.Char == ')') {
          return head;
        } else {
          throw LispException("Expect ')'.");
        }
      }
    } else {
      // Normal list element (a | b ...)
      unread();
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
  auto t = next();
  if (t.Type == TokenType::IDENT) {
    if (t.Str == "f") {
      // #f
      return BOOL_F;
    } else if (t.Str == "t") {
      // #t
      return BOOL_T;
    } else {
      throw BUG();
    }
  } else {
    throw BUG();
  }
}

} // namespace cxxlisp
