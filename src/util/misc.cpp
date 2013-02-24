#if defined(__linux__)
#include <sys/prctl.h>
#else
#include <unistd.h>
#endif
#include "util/misc.hpp"

namespace util
{

void SetProcessTitle(const std::string& title)
{
#if defined(__linux__)
  prctl(PR_SET_NAME, title.c_str(), 0, 0, 0);
#else
  setproctitle(title.c_str());
#endif
}

} /* util namespace */
