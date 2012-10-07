#ifndef __FTP_READWRITEABLE_HPP
#define __FTP_READWRITEABLE_HPP

#include <sys/types.h>

namespace ftp
{

class ReadWriteable
{
public:
  virtual ~ReadWriteable() { }
  virtual size_t Read(char* buffer, size_t size) = 0;
  virtual void Write(const char* buffer, size_t len) = 0;
};

} /* ftp namespace */

#endif
