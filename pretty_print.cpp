#include "util.hpp"
#include "value.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

static bool pp_(ostream &os, Ctx &ctx, Value v, int &len) {

  auto p = [&len, &os](string_view str) {
    if (len >= str.length()) {
      os << str;
      len -= (int)str.length();
      return true;
    } else {
      return false;
    }
  };

  switch (v.Type()) {
  case ValueType::NIL:
    return p("()");

  case ValueType::SPECIAL: {
    auto &str = v.AsSpecial();
    return p(str);
  }
  case ValueType::NUMBER: {
    string str = to_string(v.AsNumber());
    return p(str);
  }
  case ValueType::ATOM: {
    auto &str = ctx.vm->AtomToString(v.AsAtom());
    return p(str);
  }
  case ValueType::STRING: {
    auto &str = v.AsString();
    if (len >= str.length() + 2) {
      os << '"' << str << '"'; // TODO: Escape
      return (int)str.length();
    } else {
      return 0;
    }
  }
  case ValueType::PROCEDURE: {
    return p("#<proc>"); // TODO: lambda name
  }
  case ValueType::CELL: {
    if (!p("(")) {
      return false;
    }

    for (;;) {
      switch (cdr(v).Type()) {
      case ValueType::NIL:
        if (!pp_(os, ctx, car(v), len)) {
          return false;
        }
        if (!p(")")) {
          return false;
        }
        return true;
      case ValueType::CELL: {
        if (!pp_(os, ctx, car(v), len)) {
          return false;
        }
        if (!p(" ")) {
          return false;
        }
        v = cdr(v);
        continue;
      }
      default:
        if (!pp_(os, ctx, car(v), len)) {
          return false;
        }
        if (!p(" . ")) {
          return false;
        }
        if (!pp_(os, ctx, cdr(v), len)) {
          return false;
        }
        if (!p(")")) {
          return false;
        }
        return true;
      }
    }
  }
  default:
    assert(0);
  }
}

ostream &pretty_print(ostream &os, Value v, int len) {
  Ctx ctx{VM::Default, &VM::Default->RootEnv()};
  if (!pp_(os, ctx, v, len)) {
    os << "...";
  }
  return os;
}

} // namespace cxxlisp
