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

#include <sstream>
#include "util/string.hpp"
#include "exec/util.hpp"
#include "ftp/client.hpp"
#include "acl/user.hpp"

namespace exec
{

std::vector<std::string> BuildEnv(ftp::Client& client)
{
  std::ostringstream os;
  os << "USER=" << client.User().Name() << "\n"
     << "UID=" << client.User().ID() << "\n"
     << "FLAGS=" << client.User().Flags() << "\n"
     << "TAGLINE=" << client.User().Tagline() << "\n"
     << "GROUP=" << client.User().PrimaryGroup() << "\n"
     << "GID=" << client.User().PrimaryGID() << "\n"
     << "HOST=" << client.Ident() << "@" << client.IP();
  
  std::string envStr(os.str());
  std::vector<std::string> env;
  util::Split(env, envStr, "\n", true);
  return env;
}

} /* exec namespace */
