#ifndef __FTP_UTIL_HPP
#define __FTP_UTIL_HPP

#include <algorithm>
#include <vector>
#include <string>
#include <sys/types.h>
#include <boost/thread/thread.hpp>
#include "ftp/transferstate.hpp"

namespace ftp
{

void CRLFtoLF(const char* source, size_t len, std::vector<char>& dest);
void LFtoCRLF(const char* source, size_t len, std::vector<char>& dest);

inline void ASCIITranscodeRETR(const char* source, size_t len, std::vector<char>& asciiBuf)
{
  LFtoCRLF(source, len, asciiBuf);
}

inline void ASCIITranscodeSTOR(const char* source, size_t len, std::vector<char>& asciiBuf)
{
#if defined(__CYGWIN__)
  LFtoCRLF(source, len, asciiBuf);
#else
  CRLFtoLF(source, len, asciiBuf);
#endif
}

inline void StripTelnetChars(std::string& commandLine)
{
  auto it = commandLine.begin();
  for (; it != commandLine.end(); ++it)
  {
    if (static_cast<unsigned char>(*it) <= 240) break;
  }
  
  commandLine.erase(commandLine.begin(), it);
}

} /* ftp namespace */

#endif
