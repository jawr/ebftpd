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

#ifndef __DB_GROUP_HPP
#define __DB_GROUP_HPP

#include <string>
#include <memory>
#include <boost/optional/optional_fwd.hpp>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
class Query;
}

namespace acl
{
class Group;
struct GroupData;
} 

namespace db
{

class Group
{
  acl::GroupData& group;
  
  void UpdateLog() const;
  void SaveField(const std::string& field);
  
public:
  Group(acl::GroupData& group) :  group(group) { }
  
  bool Create();
  bool SaveName();
  void SaveDescription();
  void SaveComment();
  void SaveSlots();
  void SaveLeechSlots();
  void SaveAllotmentSlots();
  void SaveMaxAllotmentSize();
  void SaveMaxLogins();
  
  int NumSlotsUsed() const;
  int NumMembers() const;
  int NumLeeches() const;
  int NumAllotments() const;
  long long TotalAllotmentSize() const;

  void Purge() const;
  
  static boost::optional<acl::GroupData> Load(acl::GroupID gid);
  static boost::optional<acl::GroupData> Load(const std::string& name);
};


std::vector<acl::GroupID> GetGIDs(const std::string& multiStr = "*");
std::vector<acl::GroupData> GetGroups(const std::string& multiStr = "*");


} /* db namespace */

#endif
