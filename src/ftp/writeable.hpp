#ifndef __FTP_READWRITEABLE_HPP
#define __FTP_READWRITEABLE_HPP

#include <sys/types.h>

namespace ftp
{

class Writeable
{
public:
  virtual ~Writeable() { }
  virtual void Write(const char* buffer, size_t len) = 0;
};

} /* ftp namespace */

#endif
