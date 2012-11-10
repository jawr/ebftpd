#ifndef __DB_MAIL_MAIL_HPP
#define __DB_MAIL_MAIL_HPP

#include <vector>
#include "acl/types.hpp"

namespace db { namespace mail
{

class Message;

void Send(const Message& message);
std::vector<Message> Get(acl::UserID recipient);
bool Save(acl::UserID recipient, int index);
unsigned SaveTrash(acl::UserID recipient);
bool Purge(acl::UserID recipient, int index);
unsigned PurgeTrash(acl::UserID recipient);
void LogOffPurgeTrash(acl::UserID recipient);
void Trash(const Message& message);

} /* mail namespace */
} /* db namespace */

#endif
