//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
