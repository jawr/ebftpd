#ifndef __CFG_UTIL_HPP
#define __CFG_UTIL_HPP

#include <string>
#include <typeinfo>

namespace cfg
{

bool YesNoToBoolean(std::string s);
long long ParseSize(std::string s);

} /* cfg namespace */

#endif
