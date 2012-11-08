#ifndef __FTP_UTIL_HPP
#define __FTP_UTIL_HPP

#include <vector>
#include <sys/types.h>

namespace ftp { namespace util
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

} /* util namespace */
} /* ftp namespace */

#endif
