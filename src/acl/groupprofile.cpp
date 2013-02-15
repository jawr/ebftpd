#include <cassert>
#include "recode/acl/groupprofile.hpp"
#include "db/groupprofile.hpp"

namespace acl
{

GroupProfile::GroupProfile() :
  db(*this),
  slots(0),
  leechSlots(-2),
  allotmentSlots(-2),
  maxAllotmentSize(0),
  maxLogins(-1)
{
}

void GroupProfile::Rename(const std::string& name)
{
  this->name = name; 
  db->SaveName();
}

void GroupProfile::SetDescription(const std::string& description)
{
  this->description = description;
  db->SaveDescription();
}

void GroupProfile::SetComment(const std::string& comment)
{
  this->comment = comment;
  db->SaveComment();
}

void GroupProfile::SetSlots(int slots)
{
  this->slots = slots;
  db->SaveSlots();
}

void GroupProfile::SetLeechslots(int leechSlots)
{
  this->leechSlots = leechSlots;
  db->SaveLeechSlots();
}

void GroupProfile::SetAllotmentSlots(int allotmentSlots)
{
  this->allotmentSlots = allotmentSlots;
  db->SaveAllotmentSlots();
}

void GroupProfile::SetMaxAllotmentSize(long long maxAllotmentSize)
{
  this->maxAllotmentSize = maxAllotmentSize;
  db->SaveMaxAllotmentSize();
}

void GroupProfile::SetMaxLogins(int maxLogins)
{
  this->maxLogins = maxLogins;
  db->SaveMaxLogins();
}

boost::optional<GroupProfile> GroupProfile::Load(acl::GroupID gid)
{
  return db::GroupProfile::Load(gid);
}

GroupProfile GroupProfile::Skeleton(acl::GroupID gid)
{
  GroupProfile profile;
  profile.id = gid;
  return profile;
}

} /* acl namespace */
