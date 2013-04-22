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

#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <memory>
#include <string>
#include <vector>
#include <boost/optional/optional_fwd.hpp>
#include "acl/types.hpp"

namespace db
{
class Group;
}

namespace acl
{

struct GroupData;

class Group
{
  std::unique_ptr<GroupData> data;
  std::unique_ptr<db::Group> db;
  
  Group();
  Group(GroupData&& data_);
  
public:
  Group& operator=(Group&& rhs);
  Group& operator=(const Group& rhs);
  
  Group(Group&& other);
  Group(const Group& other);
  
  ~Group();

  acl::GroupID ID() const;
  
  const std::string& Name() const;
  bool Rename(const std::string& name);

  const std::string& Description() const;
  void SetDescription(const std::string& description);
  
  const std::string& Comment() const;
  void SetComment(const std::string& comment);

  int Slots() const;
  void SetSlots(int slots);
  
  int LeechSlots() const;
  void SetLeechSlots(int leechSlots);
  
  int AllotmentSlots() const;
  void SetAllotmentSlots(int allotmentSlots);
  
  long long MaxAllotmentSize() const;
  void SetMaxAllotmentSize(long long maxAllotmentSize);
  
  int MaxLogins() const;
  void SetMaxLogins(int maxLogins);

  int NumSlotsUsed() const;
  int NumMembers() const;
  int NumLeeches() const;
  int NumAllotments() const;
  long long TotalAllotmentSize() const;
  
  void Purge();
  
  static boost::optional<Group> Load(acl::GroupID gid);
  static boost::optional<Group> Load(const std::string& name);
  static boost::optional<Group> Create(const std::string& name);
  static boost::optional<Group> FromTemplate(const std::string& name, const Group& templateGroup);

  static std::vector<acl::GroupID> GetGIDs(const std::string& multiStr = "*");
  static std::vector<acl::Group> GetGroups(const std::string& multiStr = "*");
};

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

inline bool GIDExists(acl::GroupID gid)
{ return GIDToName(gid) != "unknown"; }

inline bool GroupExists(const std::string& name)
{ return NameToGID(name) != -1; }


} /* acl namespace */

#endif
