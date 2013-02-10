#ifndef __UTIL_FORMAT_HPP
#define __UTIL_FORMAT_HPP

#include <string>
#include <functional>
#include <boost/format.hpp>
#include "util/enumstrings.hpp"

namespace util
{

class Format
{
public:
  typedef std::function<void(const std::string& s)> OutputFunction;
  
protected:
  boost::format format;
  std::function<void(const std::string& s)> output;

  boost::format CustomFormat(const char* s)
  {
    using namespace boost::io;
    boost::format format(s);
    format.exceptions(all_error_bits ^ (too_many_args_bit | too_few_args_bit));
    return format;
  }
  
  boost::format CustomFormat(const std::string& s)
  {
    return CustomFormat(s.c_str());
  }
  
  void NextArg()
  {
    if (output) output(format.str());
  }
  
  template <typename T, typename... Args>
  void NextArg(const T& arg, Args... args)
  {
    format % arg;
    NextArg(args...);
  }
  
public:
  Format() = default;
  Format(const OutputFunction& output) : output(output) { }
  
  virtual ~Format() { }

  template <typename Enum>
  Format& operator()(Enum e)
  {
    format = CustomFormat(util::EnumToString(e));
    NextArg();
    return *this;
  }

  template <typename Enum, typename T, typename... Args>
  Format& operator()(Enum e, const T& arg, Args... args)
  {
    format = CustomFormat(util::EnumToString(e)) % arg;
    NextArg(args...);
    return *this;
  }
  
  Format& operator()(const char* s)
  {
    format = CustomFormat(s);
    NextArg();
    return *this;
  }

  template <typename T, typename... Args>
  Format& operator()(const char* s, const T& arg, Args... args)
  {
    format = CustomFormat(s) % arg;
    NextArg(args...);
    return *this;
  }
    
  Format& operator()(const std::string& s)
  {
    return operator()(s.c_str());
  }

  template <typename T, typename... Args>
  Format& operator()(const std::string& s, const T& arg, Args... args)
  {
    return operator()(s.c_str(), arg, args...);
  }

  std::string String() const { return format.str(); }  
  operator std::string() const { return format.str(); }
};

inline std::ostream& operator<<(std::ostream& os, const Format& format)
{
  return (os << format.String());
}

} /* util namespace */

#endif
