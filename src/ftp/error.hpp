#ifndef __FTP_ERROR_HPP
#define __FTP_ERROR_HPP

#include <string>
#include "util/error.hpp"

namespace ftp
{
 
class ProtocolError : util::RuntimeError
{
public:
  ProtocolError() : std::runtime_error("FTP protocol violation") { }
  ProtocolError(const std::string& message) : 
    std::runtime_error("FTP protocol violation: " + message) { }
};
 
} /* ftp namespace */

#endif
