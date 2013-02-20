#include <cassert>
#include "acl/group.hpp"
#include "db/group.hpp"
#include "util/error.hpp"
#include "db/error.hpp"
#include "db/grouputil.hpp"
#include "util/scopeguard.hpp"

namespace acl
{

GroupData::GroupData() :
  id(-1),
  slots(0),
  leechSlots(-2),
  allotmentSlots(-2),
  maxAllotmentSize(0),
  maxLogins(-1)
{
}

Group::Group() :
  db(data)
{
}

Group::Group(GroupData&& data_) :
  data(data_),
  db(data)
{
}

Group& Group::operator=(Group&& rhs)
{
  data = rhs.data;
  return *this;
}

Group& Group::operator=(const Group& rhs)
{
  data = rhs.data;
  return *this;
}

Group::Group(Group&& other) :
  data(other.data),
  db(data)
{
}

Group::Group(const Group& other) :
  data(other.data),
  db(data)
{
}

Group::~Group()
{
}

bool Group::Rename(const std::string& name)
{
  std::string oldName = data.name;
  data.name = name; 
  if (!db->SaveName())
  {
    data.name.swap(oldName);
    return false;
  }
  return true;
}

void Group::SetDescription(const std::string& description)
{
  auto trans = util::MakeTransaction(data.description, description);
  db->SaveDescription();
}

void Group::SetComment(const std::string& comment)
{
  auto trans = util::MakeTransaction(data.description, comment);
  db->SaveComment();
}

void Group::SetSlots(int slots)
{
  auto trans = util::MakeTransaction(data.slots, slots);
  db->SaveSlots();
}

void Group::SetLeechSlots(int leechSlots)
{
  auto trans = util::MakeTransaction(data.leechSlots, leechSlots);
  db->SaveLeechSlots();
}

void Group::SetAllotmentSlots(int allotmentSlots)
{
  auto trans = util::MakeTransaction(data.allotmentSlots, allotmentSlots);
  db->SaveAllotmentSlots();
}

void Group::SetMaxAllotmentSize(long long maxAllotmentSize)
{
  auto trans = util::MakeTransaction(data.maxAllotmentSize, maxAllotmentSize);
  db->SaveMaxAllotmentSize();
}

void Group::SetMaxLogins(int maxLogins)
{
  auto trans = util::MakeTransaction(data.maxLogins, maxLogins);
  db->SaveMaxLogins();
}

long long Group::NumMembers() const
{
  return db->NumMembers();
}

void Group::Purge()
{
  db->Purge();
}

boost::optional<Group> Group::Load(acl::GroupID gid)
{
  auto data = db::Group::Load(gid);
  if (!data) return boost::optional<Group>();
  return boost::optional<Group>(Group(std::move(*data)));
}

boost::optional<Group> Group::Load(const std::string& name)
{
  auto data = db::Group::Load(name);
  if (!data) return boost::optional<Group>();
  return boost::optional<Group>(Group(std::move(*data)));
}

boost::optional<Group> Group::Create(const std::string& name)
{
  Group group;
  group.data.name = name;
  if (!group.db->Create()) return boost::optional<Group>();
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
