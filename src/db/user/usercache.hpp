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

#ifndef __DB_USERCACHE_HPP
#define __DB_USERCACHE_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "acl/types.hpp"
#include "db/replicable.hpp"
#include "db/user/usercachebase.hpp"

namespace mongo
{
class BSONElement;
}

namespace db
{

class UserCache : 
  public UserCacheBase,
  public Replicable
{
  std::mutex namesMutex;
  std::unordered_map<acl::UserID, std::string> names;

  std::mutex uidsMutex;
  std::unordered_map<std::string, acl::UserID> uids;
  
  std::mutex primaryGidsMutex;
  std::unordered_map<acl::UserID, acl::GroupID> primaryGids;

  std::mutex ipMasksMutex;
  std::unordered_map<acl::UserID, std::vector<std::string>> ipMasks;
  
  std::function<void(acl::UserID)> updatedCallback;
  
public:  
  UserCache(const std::function<void(acl::UserID)>& updatedCallback) : 
    Replicable("users"),
    updatedCallback(updatedCallback)
  { }
  
  std::string UIDToName(acl::UserID uid);
  acl::UserID NameToUID(const std::string& name);
  acl::GroupID UIDToPrimaryGID(acl::UserID uid);  
  bool IdentIPAllowed(const std::string& identAddress);
  bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);

  bool Replicate(const mongo::BSONElement& id);
  bool Populate();  
};

} /* db namespace */

#endif
