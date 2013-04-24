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

#ifndef __PLUGIN_ERROR_HPP
#define __PLUGIN_ERROR_HPP

#include "util/error.hpp"

namespace plugin
{

struct PluginError : public util::RuntimeError
{
public:
  PluginError() : std::runtime_error("Unknown script error") { }
	PluginError(const std::string& message) : std::runtime_error(message) { }
};

struct InitialiseError : public PluginError
{
public:
  InitialiseError(const std::string& message) : std::runtime_error(message) { }
};

struct ValueError : public PluginError
{
public:
  ValueError() : std::runtime_error("Value error") { }
  ValueError(const std::string& message) : std::runtime_error(message) { }
};

struct NotConstructable : public PluginError
{
  NotConstructable() : std::runtime_error("Not constructible") { }
};

} /* script namespace */

#endif
