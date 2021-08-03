#include <iostream> // TODO: remove
#include <unordered_map>
#include <vector>

#include "value.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

//===================================================================
// Constants.
//===================================================================

Value NIL;
Value BOOL_T = Value::CreateSpecial("#t");
Value BOOL_F = Value::CreateSpecial("#f");
Value SYM_QUOTE = Value::CreateSpecial("quote");
Value SYM_QUASIQUOTE = Value::CreateSpecial("quasiquote");
Value SYM_UNQUOTE = Value::CreateSpecial("unquote");

const char *VALUE_TYPE_NAMES[] = {
    "NIL",  "SPECIAL", "NUMBER",    "ATOM",
    "CELL", "STRING",  "PROCEDURE", "CUSTOM_OBJECT",
};

//===================================================================
// Procedure
//===================================================================

void Procedure::check(int arity) const {
  if (arity_ != arity) {
    throw LispException("Ariry mismatch");
  }
}

//===================================================================
// Value
//===================================================================
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

const string Value::ToString(const VM *vm) const {
  if (vm == nullptr) {
    vm = VM::Default;
  }

  switch (Type()) {
  case ValueType::NIL:
    return "()";
  case ValueType::SPECIAL:
    return AsSpecial();
  case ValueType::NUMBER:
    return to_string(AsNumber());
  case ValueType::ATOM:
    return vm->AtomToString(AsAtom());
  case ValueType::CELL: {
    auto *cur = &AsCell();
    string r = "(";
    for (;;) {
      if (cur->Cdr.IsCell()) {
        r += cur->Car.ToString(vm) + " ";
        cur = &cur->Cdr.AsCell();
      } else {
        if (cur->Cdr.IsNil()) {
          r += cur->Car.ToString(vm);
        } else {
          r += cur->Car.ToString(vm) + " . " + cur->Cdr.ToString(vm);
        }
        break;
      }
    }
    return r + ")";
  }
  case ValueType::STRING:
    return '"' + AsString() + '"';
  case ValueType::PROCEDURE:
    return "#<proc>";
  default:
    return "?";
  }
}

const string Value::ToString(const VM &vm) const { return ToString(&vm); }

Value::Value(const std::string &v)
    : type_(ValueType::STRING),
      i_(reinterpret_cast<uintptr_t>(new StringValue(v))) {}

} // namespace cxxlisp
