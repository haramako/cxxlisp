#include <iostream>
#include <unordered_map>
#include <vector>

#include "value.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

bool operator==(const Value &a, const Value &b) {
  if (a.Type() != b.Type()) {
    return false;
  }

  switch (a.Type()) {
  case ValueType::NIL:
    return true;
  case ValueType::STRING:
    return a.AsString() == b.AsString();
  default:
    return a.i_ == b.i_;
  }
}

const string &Value::AsString() const { return ref<StringValue>().Ref(); }

const string Value::ToString(const VM &vm) const {
  switch (Type()) {
  case ValueType::NIL:
    return "#nil";
  case ValueType::NUMBER:
    return to_string(AsNumber());
  case ValueType::ATOM:
    return vm.AtomToString(AsAtom());
  case ValueType::CELL: {
    Cell &cur = const_cast<Cell &>(AsCell()); // TODO: const_cast がおかしい
    string r = "(";
    for (;;) {
      if (cur.Cdr.IsCell()) {
        r += cur.Car.ToString(vm) + " ";
        cur = cur.Cdr.AsCell();
      } else {
        if (cur.Cdr.IsNil()) {
          r += cur.Car.ToString(vm);
        } else {
          r += cur.Car.ToString(vm) + " . " + cur.Cdr.ToString(vm);
        }
        break;
      }
    }
    return r + ")";
  }
  case ValueType::STRING:
    return AsString();
  default:
    return "?";
  }
}

Value::Value(const std::string &v)
    : type_(ValueType::STRING),
      i_(reinterpret_cast<uintptr_t>(new StringValue(v))) {}

Value NIL;

} // namespace cxxlisp
