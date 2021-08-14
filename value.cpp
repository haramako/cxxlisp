#include <iostream> // TODO: remove
#include <unordered_map>
#include <vector>

#include "errors.hpp"
#include "util.hpp"
#include "vm.hpp"

namespace cxxlisp {

using namespace std;

//===================================================================
// Constants.
//===================================================================

Value NIL;
Value BOOL_T = Value::CreateSpecial("#t");
Value BOOL_F = Value::CreateSpecial("#f");
Value UNDEF = Value::CreateSpecial("#undef");
Value SYM_QUOTE = Value::CreateSpecialForm(SpecialForm::QUOTE);
Value SYM_QUASIQUOTE = Value::CreateSpecialForm(SpecialForm::QUASIQUOTE);
Value SYM_UNQUOTE = Value::CreateSpecialForm(SpecialForm::UNQUOTE);
Value SYM_LAMBDA = Value::CreateSpecialForm(SpecialForm::LAMBDA);
Value SYM_UNQUOTE_SPLICING =
    Value::CreateSpecialForm(SpecialForm::UNQUOTE_SPLICING);

const char *VALUE_TYPE_NAMES[] = {
    "NIL",  "SPECIAL", "NUMBER",    "ATOM",
    "CELL", "STRING",  "PROCEDURE", "CUSTOM_OBJECT",
};

//===================================================================
// Procedure
//===================================================================

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

const string &Value::AsString() const {
  chk(ValueType::STRING);
  return ref<StringValue>().Ref();
}

const string Value::ToString(const VM *vm) const {
  stringstream s;
  pretty_print(s, *vm, *this);
  return s.str();
}

const string Value::ToString(const VM &vm) const { return ToString(&vm); }

Value::Value(const std::string &v)
    : type_(ValueType::STRING),
      i_(reinterpret_cast<uintptr_t>(new StringValue(v))) {}

} // namespace cxxlisp
