#ifndef __DB_MAIL_MAIL_HPP
#define __DB_MAIL_MAIL_HPP

#include <vector>
#include "acl/types.hpp"

namespace db { namespace mail
{

class Message;

void Send(const Message& message);
std::vector<Message> Get(acl::UserID recipient);

} /* mail namespace */
} /* db namespace */

#endif
