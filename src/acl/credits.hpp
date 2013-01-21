#ifndef __ACL_CREDITS_HPP
#define __ACL_CREDITS_HPP

#include <boost/optional.hpp>

namespace cfg { namespace setting
{

class Creditcheck;
class Creditloss;

}
}

namespace fs
{
class VirtualPath;
}

namespace acl
{

class User;

boost::optional<const cfg::setting::Creditcheck&> 
CreditCheck(const User& user, const fs::VirtualPath& path);

boost::optional<const cfg::setting::Creditloss&> 
CreditLoss(const User& user, const fs::VirtualPath& path);

} /* acl namespace */

#endif
