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

#ifndef __EXEC_CSCRIPT_HPP
#define __EXEC_CSCRIPT_HPP

#include <string>
#include "cfg/get.hpp"
#include "ftp/replycodes.hpp"

namespace ftp
{

class Client;

}

namespace exec
{

typedef cfg::Cscript::Type CscriptType;

bool Cscripts(ftp::Client& client, const std::string& command, 
    const std::string& fullCommand, CscriptType type, ftp::ReplyCode failCode);

} /* exec namespace */

#endif

