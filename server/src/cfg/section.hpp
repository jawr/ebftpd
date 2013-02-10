#ifndef __CFG_SECTION_HPP
#define __CFG_SECTION_HPP

#include <string>
#include <vector>

namespace fs
{
class Path;
}

namespace cfg
{

class Section
{
  std::string name;
  std::vector<std::string> paths;
  bool separateCredits;
  int ratio;

public:
  Section(const std::string& name) :
    name(name),
    separateCredits(false),
    ratio(-1)
  { }
  
  const std::string& Name() const { return name; }
  bool IsMatch(const std::string& path) const;
  bool SeparateCredits() const { return separateCredits; }
  int Ratio() const { return ratio; }
  
  friend class Config;
};

} /* cfg namespace */

#endif
