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

#ifndef __CMD_ERROR_HPP
#define __CMD_ERROR_HPP

#include "util/error.hpp"

namespace cmd
{

struct SyntaxError : public util::RuntimeError
{
  SyntaxError() : std::runtime_error("Syntax error.") { }
};

struct PermissionError : public util::RuntimeError
{
  PermissionError() : std::runtime_error("Permission error.") { }
};

struct NoPostScriptError : public util::RuntimeError
{
  NoPostScriptError() : std::runtime_error("Don't run post cscript.") { }
};

} /* cmd namespace */

#endif
