#include "acl/recode/group.hpp"
#include "acl/recode/groupprofile.hpp"

namespace acl { namespace recode
{

void GroupUpdatedSlot(const std::function<void(acl::GroupID)>& slot)
{
  GroupDBProxy::ConnectUpdatedSlot(slot);
  GroupProfileDBProxy::ConnectUpdatedSlot(slot);
}

Group::Group() : 
  db(*this), id(-1)
{
}

Group::Group(const std::string& name) :
  db(*this), id(-1), name(name)
{
}

Group::~Group()
{
}

void Group::Rename(const std::string& name)
{
  this->name = name; 
  db->SaveName();
}

} /* recode namespace */
} /* acl namespace */
