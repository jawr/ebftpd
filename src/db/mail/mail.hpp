#ifndef __DB_MAIL_MAIL_HPP
#define __DB_MAIL_MAIL_HPP

#include <vector>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
}

namespace db { namespace mail
{

class Message;

void Send(const Message& message);
std::vector<Message> Get(acl::UserID recipient);
bool Save(acl::UserID recipient, int index);
int SaveTrash(acl::UserID recipient);
bool Purge(acl::UserID recipient, int index);
int PurgeTrash(acl::UserID recipient);
void LogOffPurgeTrash(acl::UserID recipient);
void Trash(const Message& message);

mongo::BSONObj Serialize(const Message& message);
Message Unserialize(const mongo::BSONObj& obj);

} /* mail namespace */
} /* db namespace */

#endif
