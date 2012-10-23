#ifndef __DB_USER_USERPROFILE_HPP
#define __DB_USER_USERPROFILE_HPP

namespace db { namespace userprofile
{

  void Save(const acl::UserProfile& profile);
  void GetAll(std::vector<acl::UserProfile*>& profiles);
  acl::UserProfile* Get(const acl::UserID& uid);

// end
}
}
#endif
