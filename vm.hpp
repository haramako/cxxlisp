#pragma once
#include <unordered_map>
#include <vector>

#include "value.hpp"

namespace cxxlisp {

class VM {
  std::unordered_map<std::string, Atom> atomKeyToId_;
  std::vector<std::string> atomIdToKey_;

public:
  VM() {}
  VM(const VM &) = delete;
  VM &operator=(const VM &) = delete;

  Atom Intern(const char *v);
  Atom Intern(const std::string &v) { return Intern(v.c_str()); }
  const std::string &AtomToString(Atom atom) { return atomIdToKey_[atom.Id()]; }
};

} // namespace cxxlisp
