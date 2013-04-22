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

#ifndef __CFG_EXCEPTION_HPP
#define __CFG_EXCEPTION_HPP

#include "util/error.hpp"

namespace cfg
{

struct ConfigError : public util::RuntimeError
{
  ConfigError() : std::runtime_error("Config error.") { }
  ConfigError(const std::string& message) : std::runtime_error(message) { }
};

struct RequiredSettingError : public ConfigError
{
  RequiredSettingError() : std::runtime_error("Missing required setting.") { }
  RequiredSettingError(const std::string& setting) : std::runtime_error("Missing required setting: " + setting) { }
};


struct StopStartNeeded : public ConfigError
{
  StopStartNeeded(const std::string& message) : std::runtime_error(message) { }
};

}

#endif
