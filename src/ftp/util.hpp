#ifndef __FTP_UTIL_HPP
#define __FTP_UTIL_HPP

#include <vector>
#include <sys/types.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include "ftp/transferstate.hpp"

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

inline void SpeedLimitSleep(const TransferState& state, int maxSpeed)
{
  namespace pt = boost::posix_time;
  pt::time_duration elapsed = 
      pt::microsec_clock::local_time() - state.StartTime();
  pt::time_duration minElapsed = pt::microseconds((state.Bytes()  / 
      1024.0 / maxSpeed) * 1000000);
  boost::this_thread::sleep(minElapsed - elapsed);
}

} /* util namespace */
} /* ftp namespace */

#endif
