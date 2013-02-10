#include "cfg/section.hpp"
#include "util/string.hpp"

namespace cfg
{

bool Section::IsMatch(const std::string& path) const
{
  for (const auto& p : paths)
  {
    if (util::string::WildcardMatch(p, path)) return true;
  }
  return false;
}

} /* cfg namespace */
