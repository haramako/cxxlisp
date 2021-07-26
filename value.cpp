#include <unordered_map>
#include <vector>

#include "value.hpp"

namespace cxxlisp {

using namespace std;

bool operator==(const Value &a, const Value &b) {
  if (a.Type() != b.Type()) {
    return false;
  }

  switch (a.Type()) {
  case ValueType::NIL:
    return true;
  case ValueType::NUMBER:
  case ValueType::ATOM:
    return a.i_ == b.i_;
  case ValueType::STRING:
    return *((string *)a.p_) == *((string *)b.p_);
  default:
    return a.p_ == b.p_;
  }
}

const string &Value::AsString() const { return ((StringValue *)p_)->Ref(); }

Value::Value(const std::string &v)
    : type_(ValueType::STRING), p_(new StringValue(v)) {}

// std::string Cell::str() : { return ""; }

Value NIL;

} // namespace cxxlisp
