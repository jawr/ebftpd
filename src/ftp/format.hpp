#ifndef __FTP_FORMAT_HPP
#define __FTP_FORMAT_HPP

#include <functional>
#include <boost/bind.hpp>
#include "ftp/replycodes.hpp"
#include "util/format.hpp"

namespace ftp
{

class Format
{
public:
  typedef std::function<void(ReplyCode, const std::string&)> ReplyFunction;
  
private:
  ReplyCode code;
  util::Format format;
  
  Format& operator=(const Format&&) = delete;
  Format& operator=(const Format&) = delete;
  Format(Format&&) = delete;
  Format(const Format&) = delete;
  
public:
  Format(const ReplyFunction& replyFunction) :
    format(boost::bind(replyFunction, boost::ref(code), _1))
  { }
  
  template <typename Enum>
  Format& operator()(ReplyCode code, Enum e)
  {
    this->code = code;
    format(e);
    return *this;
  }

  template <typename Enum, typename... Args>
  Format& operator()(ReplyCode code, Enum e, const Args&... args)
  {
    this->code = code;
    format(e, args...);
    return *this;
  }
  
  Format& operator()(ReplyCode code, const char* s)
  {
    this->code = code;
    format(s);
    return *this;
  }

  template <typename... Args>
  Format& operator()(ReplyCode code, const char* s, const Args&... args)
  {
    this->code = code;
    format(s, args...);
    return *this;
  }
    
  Format& operator()(ReplyCode code, const std::string& s)
  {
    this->code = code;
    format(s.c_str());
    return *this;
  }

  template <typename... Args>
  Format& operator()(ReplyCode code, const std::string& s, const Args&... args)
  { 
    this->code = code;
    format(s.c_str(), args...);
    return *this;
  }
};

} /* ftp namespace */

#endif
