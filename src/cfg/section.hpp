//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
