#ifndef __ACL_ALLOWFXP_HPP
#define __ACL_ALLOWFXP_HPP

#include <cstdint>
#include "ftp/data.hpp"

namespace ftp
{
enum class TransferType : uint8_t;
}

namespace acl
{

class User;

bool AllowFxp(ftp::TransferType transferType,
              const User& user, bool& logging);

} /* acl namespace */

#endif
