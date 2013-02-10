#ifndef __FS_MODE_HPP
#define __FS_MODE_HPP

#include <string>
#include <sys/types.h>
#include <vector>
#include "util/error.hpp"

namespace fs
{

class InvalidModeString : public util::RuntimeError
{
public:
  InvalidModeString() : std::runtime_error("Invalid mode string.") { }
};

class Mode
{
  enum class Type : int { Normal, Copy };
  enum class Operator : char { Plus = '+', Minus = '-', Equals = '=' };
  
  class Change
  {
    Operator op;
    ::fs::Mode::Type type;
    mode_t affected;
    mode_t value;

  public:
    Change(Operator op, ::fs::Mode::Type type, mode_t affected, mode_t value) :
      op(op), type(type), affected(affected), value(value) { }
      
    Operator Op() const { return op; }
    ::fs::Mode::Type Type() const { return type; }
    mode_t Affected() const { return affected; }
    mode_t Value() const { return value; }
  };
  
  typedef std::vector<Change> Changes;
  Changes changes;
  
  void CompileNumericMode(const std::string& str);
  void CompileSymbolicMode(const std::string& str);
  void Compile(const std::string& str);
  
public:
  Mode(const std::string& str) { Compile(str); }
  
  void Apply(mode_t oldMode, mode_t umask, mode_t& newMode) const;
};

} /* fs namespace */

#endif
