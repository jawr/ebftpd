#include <cassert>
#include "acl/group.hpp"
#include "db/group.hpp"
#include "util/error.hpp"
#include "db/error.hpp"
#include "db/grouputil.hpp"

namespace acl
{

Group::Group() :
  db(*this),
  id(-1),
  slots(0),
  leechSlots(-2),
  allotmentSlots(-2),
  maxAllotmentSize(0),
  maxLogins(-1)
{
}

Group::~Group()
{
}

bool Group::Rename(const std::string& name)
{
  std::string oldName = this->name;
  this->name = name; 
  if (!db->SaveName())
  {
    this->name.swap(oldName);
    return false;
  }
  return true;
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

void Group::SetLeechSlots(int leechSlots)
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
  return db::Group::Load(gid);
}

boost::optional<Group> Group::Load(const std::string& name)
{
  return Load(NameToGID(name));
}

boost::optional<Group> Group::Create(const std::string& name)
{
  try
  {
    Group group;
    group.name = name;
    group.id = group.db->Create();
    return boost::optional<Group>(group);
  }
  catch (const db::DBKeyError&)
  {
    return boost::optional<Group>();
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
