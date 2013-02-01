#ifndef __ACL_ALLOWFXP_HPP
#define __ACL_ALLOWFXP_HPP

namespace acl
{

class User;

bool AllowFxpSend(const User& user, bool& logging);
bool AllowFxpReceive(const User& user, bool& logging);

} /* acl namespace */

#endif
