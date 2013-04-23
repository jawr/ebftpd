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

#ifndef __TEXT_TEMPLATESECTION_HPP
#define __TEXT_TEMPLATESECTION_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <boost/logic/tribool.hpp>
#include "text/tag.hpp"
#include "util/error.hpp"

namespace text
{

class TemplateSection
{
  std::string buffer;
  std::vector<Tag> tags;
  bool isFoot;

  void DoRegisterValue(std::string tagName, const std::function<void(Tag&)>& doRegister);
  
public:
  TemplateSection(bool isFoot) : isFoot(isFoot) { }

  void RegisterBuffer(const std::string& buffer) { this->buffer = buffer; }
  
  std::string RegisterTag(std::string tagStr);

  void RegisterValue(const std::string& tagName, const std::string& value);
  void RegisterValue(const std::string& tagName, int value);

  void RegisterSize(const std::string& tagName, long long kBytes);
  void RegisterSpeed(const std::string& tagName, long long bytes, long long xfertime);
  void RegisterSpeed(const std::string& tagName, double speed);

  std::string Compile(boost::tribool stripNewline = boost::indeterminate);
  
  bool HasTag(const std::string& tagName) const
  {
    return std::find_if(tags.begin(), tags.end(), 
              [&](const Tag& tag) 
              { 
                return tag.Name() == tagName; 
              }) != tags.end();
  }
};

// end
}
#endif
