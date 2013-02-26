#ifndef __STATS_COMPILE_HPP
#define __STATS_COMPILE_HPP

#include <string>
#include <functional>
#include "stats/types.hpp"

namespace text
{
class Template;
}

namespace acl
{
class User;
}

namespace stats
{

std::string CompileUserRanks(const std::string& section, Timeframe tf, Direction dir, SortField sf, int max, 
                             text::Template& templ, const std::function<bool(acl::User&)>& filter = nullptr);
std::string CompileGroupRanks(const std::string& section, Timeframe tf, Direction dir, SortField sf, int max, 
                              text::Template& templ);

} /* stats namespace */

#endif
