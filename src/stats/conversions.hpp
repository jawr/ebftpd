#ifndef __STATS_CONVERSIONS_HPP
#define __STATS_CONVERSIONS_HPP

#include <string>
#include "stats/stat.hpp"

namespace stats
{

namespace tostring
{

std::string Kbyte(const Stat& stat);
std::string Mbyte(const Stat& stat);
std::string Gbyte(const Stat& stat);

}

// end
}
#endif
