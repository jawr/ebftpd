#ifndef __UTIL_DAEMONISE_HPP
#define __UTIL_DAEMONISE_HPP

#include <string>
#include <utility>
#include "util/error.hpp"

namespace util { namespace daemonise
{

util::Error NotRunning(const std::string& pidFile);
util::Error CreatePIDFile(const std::string& pidFile);
util::Error Daemonise();

} /* daemonise namespace */
} /* util namespace */

#endif
