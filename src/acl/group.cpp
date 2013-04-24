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

#include <cassert>
#include "acl/group.hpp"
#include "db/group/group.hpp"
#include "util/error.hpp"
#include "db/error.hpp"
#include "db/group/util.hpp"
#include "util/scopeguard.hpp"
#include "acl/groupdata.hpp"

namespace acl
{

Group::Group() :
  data(new GroupData()),
  db(new db::Group(*data))
{
}

Group::Group(GroupData&& data_) :
  data(new GroupData(data_)),
  db(new db::Group(*data))
{
}

Group& Group::operator=(Group&& rhs)
{
  data = std::move(rhs.data);
  db = std::move(rhs.db);
  return *this;
}

Group& Group::operator=(const Group& rhs)
{
  data.reset(new GroupData(*rhs.data));
  db.reset(new db::Group(*data));
  return *this;
}

Group::Group(Group&& other) :
  data(std::move(other.data)),
  db(new db::Group(*data))
{
}

Group::Group(const Group& other) :
  data(new GroupData(*other.data)),
  db(new db::Group(*data))
{
}

Group::~Group()
{
}

acl::GroupID Group::ID() const
{
  return data->id;
}

const std::string& Group::Name() const
{
  return data->name;
}

const std::string& Group::Description() const
{
  return data->description;
}

const std::string& Group::Comment() const
{
  return data->comment;
}

int Group::Slots() const
{
  return data->slots;
}

int Group::LeechSlots() const
{
  return data->leechSlots;
}

int Group::AllotmentSlots() const
{
  return data->allotmentSlots;
}

long long Group::MaxAllotmentSize() const
{
  return data->maxAllotmentSize;
}

int Group::MaxLogins() const
{
  return data->maxLogins;
}

bool Group::Rename(const std::string& name)
{
  std::string oldName = data->name;
  data->name = name; 
  if (!db->SaveName())
  {
    data->name.swap(oldName);
    return false;
  }
  return true;
}

void Group::SetDescription(const std::string& description)
{
  auto trans = util::MakeTransaction(data->description, description);
  db->SaveDescription();
}

void Group::SetComment(const std::string& comment)
{
  auto trans = util::MakeTransaction(data->description, comment);
  db->SaveComment();
}

void Group::SetSlots(int slots)
{
  auto trans = util::MakeTransaction(data->slots, slots);
  db->SaveSlots();
}

void Group::SetLeechSlots(int leechSlots)
{
  auto trans = util::MakeTransaction(data->leechSlots, leechSlots);
  db->SaveLeechSlots();
}

void Group::SetAllotmentSlots(int allotmentSlots)
{
  auto trans = util::MakeTransaction(data->allotmentSlots, allotmentSlots);
  db->SaveAllotmentSlots();
}

void Group::SetMaxAllotmentSize(long long maxAllotmentSize)
{
  auto trans = util::MakeTransaction(data->maxAllotmentSize, maxAllotmentSize);
  db->SaveMaxAllotmentSize();
}

void Group::SetMaxLogins(int maxLogins)
{
  auto trans = util::MakeTransaction(data->maxLogins, maxLogins);
  db->SaveMaxLogins();
}

int Group::NumSlotsUsed() const
{
  return db->NumSlotsUsed();
}

int Group::NumMembers() const
{
  return db->NumMembers();
}

int Group::NumLeeches() const
{
  return db->NumLeeches();
}

int Group::NumAllotments() const
{
  return db->NumAllotments();
}

long long Group::TotalAllotmentSize() const
{
  return db->TotalAllotmentSize();
}

void Group::Purge()
{
  db->Purge();
}

boost::optional<Group> Group::Load(acl::GroupID gid)
{
  auto data = db::Group::Load(gid);
  if (!data) return boost::none;
  return boost::optional<Group>(Group(std::move(*data)));
}

boost::optional<Group> Group::Load(const std::string& name)
{
  auto data = db::Group::Load(name);
  if (!data) return boost::none;
  return boost::optional<Group>(Group(std::move(*data)));
}

boost::optional<Group> Group::Create(const std::string& name)
{
  Group group;
  group.data->name = name;
  if (!group.db->Create()) return boost::none;
  return boost::optional<Group>(group);
}

boost::optional<Group> Group::FromTemplate(const std::string& name, const Group& templateGroup)
{
  Group group(templateGroup);
  group.data->name = name;
  if (!group.db->Create()) return boost::none;
  return boost::optional<Group>(group);
}

std::vector<acl::GroupID> Group::GetGIDs(const std::string& multiStr)
{
  return db::GetGIDs(multiStr);
}

std::vector<acl::Group> Group::GetGroups(const std::string& multiStr)
{
  auto groupData = db::GetGroups(multiStr);
  std::vector<acl::Group> groups;
  groups.reserve(groupData.size());
  for (auto& data : groupData)
  {
    groups.push_back(Group(std::move(data)));
  }
  return groups;
}

std::string GIDToName(acl::GroupID gid)
{
  return db::GIDToName(gid);
}

acl::GroupID NameToGID(const std::string& name)
{
  return db::NameToGID(name);
}

} /* acl namespace */
