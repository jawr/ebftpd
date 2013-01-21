#include "cfg/section.hpp"
#include "util/string.hpp"
#include "fs/path.hpp"

namespace cfg
{

bool Section::IsMatch(const fs::Path& path) const
{
  for (const auto& p : paths)
  {
    if (util::string::WildcardMatch(p.ToString(), path.ToString())) return true;
  }
  return false;
}

} /* cfg namespace */
