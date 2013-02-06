#include <cassert>
#include "acl/recode/groupprofile.hpp"
#include "acl/recode/groupprofiledb.hpp"

namespace acl { namespace recode
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

GroupProfile::GroupProfile(const Group& group) :
  Group(group),
  db(*this),
  slots(0),
  leechSlots(-2),
  allotmentSlots(-2),
  maxAllotmentSize(0),
  maxLogins(-1)
{
}

GroupProfile::~GroupProfile()
{
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

} /* recode namespace */
} /* acl namespace */
