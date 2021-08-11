#pragma once
#include <exception>
#include <string>

namespace cxxlisp {

class Ctx;

/**
 * Base class of lisp exceptions.
 */
class LispException : public std::exception {
  std::string msg_;

public:
  LispException(const char *msg) : msg_(msg) {}
  LispException(const std::string_view msg) : msg_(msg) {}
  const char *what() const noexcept override { return msg_.c_str(); }

  std::vector<std::string> Stack;
};

/**
 * Base class of lisp exceptions.
 */
class BUG : public LispException {
public:
  BUG() : LispException("BUG") {}
};

} // namespace cxxlisp
