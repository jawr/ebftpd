#ifndef __DB_USER_USER_HPP
#define __DB_USER_USER_HPP

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"
#include "acl/userprofile.hpp"
#include "acl/ipmaskcache.hpp"
#include "util/error.hpp"
#include "acl/user.hpp"

namespace db { namespace user
{

bool Create(acl::User& user);
void Save(const acl::User& user);    
void Save(const acl::User& user, const std::string& field);
void Delete(acl::UserID uid);
boost::ptr_vector<acl::User> GetAllPtr(const boost::optional<boost::posix_time::ptime>& modified = boost::none);
std::vector<acl::User> GetAll();
void Login(acl::UserID user);
std::vector<acl::User> GetByACL(std::string acl); 

// end
}
}
#endif
