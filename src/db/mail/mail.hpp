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

} /* mail namespace */
/*
template <> mongo::BSONObj Serialize<mail::Message>(const mail::Message& message);
template <> mail::Message Unserialize<mail::Message>(const mongo::BSONObj& obj);
*/
} /* db namespace */

#endif
