#include <cassert>
#include "acl/group.hpp"
#include "db/group.hpp"
#include "db/grouputil.hpp"
#include "util/error.hpp"

namespace acl
{

Group::Group() :
  db(*this),
  slots(0),
  leechSlots(-2),
  allotmentSlots(-2),
  maxAllotmentSize(0),
  maxLogins(-1)
{
}

void Group::Rename(const std::string& name)
{
  this->name = name; 
  db->SaveName();
}

void Group::SetDescription(const std::string& description)
{
  this->description = description;
  db->SaveDescription();
}

void Group::SetComment(const std::string& comment)
{
  this->comment = comment;
  db->SaveComment();
}

void Group::SetSlots(int slots)
{
  this->slots = slots;
  db->SaveSlots();
}

void Group::SetLeechslots(int leechSlots)
{
  this->leechSlots = leechSlots;
  db->SaveLeechSlots();
}

void Group::SetAllotmentSlots(int allotmentSlots)
{
  this->allotmentSlots = allotmentSlots;
  db->SaveAllotmentSlots();
}

void Group::SetMaxAllotmentSize(long long maxAllotmentSize)
{
  this->maxAllotmentSize = maxAllotmentSize;
  db->SaveMaxAllotmentSize();
}

void Group::SetMaxLogins(int maxLogins)
{
  this->maxLogins = maxLogins;
  db->SaveMaxLogins();
}

boost::optional<Group> Group::Load(acl::GroupID gid)
{
  return db::Group::Load(gid);
}

Group Group::Skeleton(acl::GroupID gid)
{
  Group group;
  group.id = gid;
  return group;
}

Group Group::Create(const std::string& name)
{
  try
  {
    Group group;
    group.name = name;
    group.id = group.db->Create();
    return group;
  }
  catch (const db::DBKeyError&)
  {
    throw util::RuntimeError("Group already exists");
  }
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
