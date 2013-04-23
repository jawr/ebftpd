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

#include "util/string.hpp"
#include <boost/algorithm/string/replace.hpp>
#include "text/templatesection.hpp"
#include "text/error.hpp"

namespace text
{

std::string TemplateSection::RegisterTag(std::string tagStr)
{
  util::Trim(tagStr);
  util::ToLower(tagStr);

  std::vector<std::string> args;
  util::Split(args, tagStr, "|");
  if (args.empty()) throw TemplateError("Invalid tag: " + tagStr);
  
  const std::string& name = args.front();
  Tag tag(name);

  for (auto it = args.begin() + 1; it != args.end(); ++it)
  {
    tag.SetFilter(*it);
  }

  tags.emplace_back(tag);

  return name;
}

void TemplateSection::DoRegisterValue(std::string tagName, 
    const std::function<void(Tag&)>& doRegister)
{
  util::ToLower(tagName);
  for (auto& tag : tags)
  {
    if (tag.Name() == tagName)
      doRegister(tag);
  }
}
  
void TemplateSection::RegisterValue(const std::string& tagName, const std::string& value)
{
  DoRegisterValue(tagName, [&value](Tag& tag) { tag.RegisterValue(value); });
}

void TemplateSection::RegisterValue(const std::string& tagName, int value)
{
  DoRegisterValue(tagName, [value](Tag& tag) { tag.RegisterValue(value); });
}

void TemplateSection::RegisterSize(const std::string& tagName, long long kBytes)
{
  DoRegisterValue(tagName, [kBytes](Tag& tag) { tag.RegisterSize(kBytes); });
}


void TemplateSection::RegisterSpeed(const std::string& tagName, double speed)
{
  DoRegisterValue(tagName, [speed](Tag& tag) { tag.RegisterSpeed(speed); });
}

void TemplateSection::RegisterSpeed(const std::string& tagName, long long kBytes, 
  long long xfertime)
{
  if (xfertime == 0) RegisterSpeed(tagName, kBytes);
  else RegisterSpeed(tagName, kBytes / xfertime / 1.0);
}

std::string TemplateSection::Compile(boost::tribool stripNewline)
{
  if (boost::indeterminate(stripNewline)) stripNewline = isFoot;
  std::string compiled = buffer;

  for (auto& tag : tags)
  {
    boost::replace_first(compiled, "{{" + tag.Name() + "}}", tag.Compile());
  }

  if (stripNewline) util::TrimRightIf(compiled, "\n");
  return compiled;
}

}

