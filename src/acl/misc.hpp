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

#ifndef __ACL_MISC_HPP
#define __ACL_MISC_HPP

#include <string>
#include <boost/optional/optional_fwd.hpp>
#include "acl/types.hpp"

namespace cfg
{
class SpeedLimit;
class Creditcheck;
class Creditloss;
}

namespace fs
{
class VirtualPath;
class Path;
}

namespace acl
{

class User;

namespace message
{

enum Type
{
  Welcome,
  Goodbye
};

template <Type type>
fs::Path Choose(const User& user);

} /* message namespace */

namespace stats
{

int MaxUsers(const User& user);
int MaxGroups(const User& user);

} /* stats namespace */

namespace speed
{

std::vector<const cfg::SpeedLimit*>
UploadMaximum(const User& user, const fs::Path& path);

std::vector<const cfg::SpeedLimit*>
DownloadMaximum(const User& user, const fs::Path& path);

int UploadMinimum(const User& user, const fs::Path& path);

int DownloadMinimum(const User& user, const fs::Path& path);

} /* speed namespace */


bool AllowFxpSend(const User& user, bool& logging);
bool AllowFxpReceive(const User& user, bool& logging);
bool AllowSiteCmd(const User& user, const std::string& keyword);

boost::optional<const cfg::Creditcheck&> 
CreditCheck(const User& user, const fs::VirtualPath& path);

boost::optional<const cfg::Creditloss&> 
CreditLoss(const User& user, const fs::VirtualPath& path);

class IPStrength;
bool SecureIP(const User& user, const std::string& ip, IPStrength& minimum);

class PasswdStrength;

bool SecurePass(const User& user, const std::string& password, PasswdStrength& minimum);

bool IPAllowed(const std::string& address);
bool IdentIPAllowed(UserID uid, const std::string& identAddress);


} /* acl namespace */

#endif
