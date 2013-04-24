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

#ifndef __TEXT_ERROR_HPP
#define __TEXT_ERROR_HPP

#include <sstream>
#include "util/error.hpp"

namespace text
{

struct TemplateError : public util::RuntimeError
{
  TemplateError() : std::runtime_error("Template Error.") { }
  TemplateError(const std::string& message) : std::runtime_error(message) { }
};

struct TemplateDuplicateValue : public TemplateError
{
  TemplateDuplicateValue(const std::string& message) : std::runtime_error(message) { }
};

struct TemplateFilterMalform : public TemplateError
{
  TemplateFilterMalform(const std::string& message) : std::runtime_error(message) { }
};

struct TemplateNoTag : public TemplateError
{
  TemplateNoTag(const std::string& message) : std::runtime_error(message) { }
};

class TemplateMalform : public TemplateError
{
  static std::string Format(int line, int i, const std::string& msg = "")
  {
    std::ostringstream os;
    os << "Error parsing template (Line: " << line << ", Char: " << i << "). "
      << msg ;
    return os.str();
  }
public:
  TemplateMalform(int line, int i, const std::string& msg) :
    std::runtime_error(Format(line, i, msg)) { }
  TemplateMalform(int line, int i) :
    std::runtime_error(Format(line, i)) { }
};

// end
}
#endif
