#ifndef __CFG_SECTION_HPP
#define __CFG_SECTION_HPP

#include <string>

namespace fs
{
class Path;
}

namespace cfg
{

class Section
{
  std::string name;
  std::vector<fs::Path> paths;
  bool separateCredits;

public:
  Section(const std::string& name) :
    name(name),
    separateCredits(false)
  { }
  
  const std::string& Name() const { return name; }
  bool IsMatch(const fs::Path& path) const;
  bool SeparateCredits() const { return separateCredits; }
  
  friend class Config;
};

} /* cfg namespace */

#endif
