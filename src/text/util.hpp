#ifndef __TEXT_UTIL_HPP
#define __TEXT_UTIL_HPP

#include <string>
#include "util/error.hpp"

namespace fs
{
class Path;
}

namespace text
{

util::Error GenericTemplate(const std::string& name, std::string& messages);

} /* text namespace */

#endif
