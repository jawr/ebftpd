#include "acl/recode/user.hpp"
#include "acl/recode/userprofile.hpp"

namespace acl { namespace recode
{

void UserUpdatedSlot(const std::function<void(acl::UserID)>& slot)
{
  UserDBProxy::ConnectUpdatedSlot(slot);
  UserProfileDBProxy::ConnectUpdatedSlot(slot);
}

User::User() : 
  db(*this), id(-1)
{
}

User::User(const std::string& name) :
  db(*this), id(-1), name(name)
{
}

User::~User()
{
}

void User::Rename(const std::string& name)
{
  this->name = name; 
  db->SaveName();
}

void User::AddIPMask(const std::string& ipMask)
{
  ipMasks.push_back(ipMask);
  db->AddIPMask(ipMask);
}

void User::DelIPMask(const std::string& ipMask)
{
  auto it = std::find(ipMasks.begin(), ipMasks.end(), ipMask);
  if (it != ipMasks.end())
  {
    ipMasks.erase(it);
    db->DelIPMask(ipMask);
  }
}

} /* recode namespace */
} /* acl namespace */
