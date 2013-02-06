#include "acl/recode/user.hpp"
#include "acl/recode/userprofile.hpp"
#include "acl/recode/group.hpp"
#include "acl/recode/groupprofile.hpp"

void Updated(acl::UserID uid)
{
  std::cout << "updated " << uid << std::endl;
}

int main()
{
  using namespace acl::recode;
  
  User user("biohazard");
  
  UserUpdatedSlot(Updated);
  
  std::cout << user.ID() << std::endl;
  user.Rename("someone");
  std::cout << user.Name() << std::endl;
  
  user.AddIPMask("mask");
  
  UserProfile profile(user);
  
  std::cout << profile.Name() << std::endl;
  
  profile.Rename("w00t");
  profile.Tagline("some tag");

  Group group("group");

  group.Rename("somegroup");

  GroupProfile gprofile(group);

  gprofile.SetDescription("new description");
  gprofile.SetSlots(1111);
  
 // User user(profile);
}
