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

#ifndef __DB_USERP_HPP
#define __DB_USERP_HPP

#include <string>
#include <memory>
#include <vector>
#include <boost/optional.hpp>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
}

namespace acl
{
struct UserData;
class User;
} 

namespace db
{

struct UserCacheBase;

class User
{
  acl::UserData& user;

  void UpdateLog() const;
  void SaveField(const std::string& field, bool updateLog = true) const;
  
public:
  User(acl::UserData& user) :  user(user) { }
  
  bool Create();
  bool SaveName();
  void SaveIPMasks();
  void SavePassword();
  void SaveFlags();
  void SaveGIDs();
  void SaveGadminGIDs();
  void SaveWeeklyAllotment();
  void SaveHomeDir();
  void SaveIdleTime();
  void SaveExpires();
  void SaveNumLogins();
  void SaveComment();
  void SaveTagline();
  void SaveMaxDownSpeed();
  void SaveMaxUpSpeed();
  void SaveMaxSimDown();
  void SaveMaxSimUp();
  void SaveLoggedIn();
  void SaveRatio();
  void IncrCredits(const std::string& section, long long kBytes);
  bool DecrCredits(const std::string& section, long long kBytes, bool force);
  
  void Purge() const;
  
  static boost::optional<acl::UserData> Load(acl::UserID uid);
  static boost::optional<acl::UserData> Load(const std::string& name);
};

std::vector<acl::UserID> GetUIDs(const std::string& multiStr = "*");
std::vector<acl::UserData> GetUsers(const std::string& multiStr = "*");

} /* db namespace */

#endif
