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

#ifndef __TEXT_UTIL_HPP
#define __TEXT_UTIL_HPP

#include <string>
#include "util/error.hpp"

namespace fs
{
class Path;
}

namespace ftp
{
class Client;
}

namespace text
{
class TemplateSection;
class Template;

void RegisterGlobals(const ftp::Client& client, TemplateSection& ts);
util::Error GenericTemplate(const ftp::Client& client, Template& tmpl, std::string& messages);
util::Error GenericTemplate(const ftp::Client& client, const std::string& name, std::string& messages);
util::Error GenericTemplate(const ftp::Client& client, const fs::Path& path, std::string& messages);
      
} /* text namespace */

#endif
