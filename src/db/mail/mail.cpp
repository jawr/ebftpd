#include "db/mail/mail.hpp"
#include "db/mail/message.hpp"
#include "db/bson/message.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/types.hpp"

namespace db { namespace mail
{

void Send(const Message& message)
{
  mongo::BSONObj obj = db::bson::Message::Serialize(message);
  logs::debug << obj.toString() << logs::endl;
  TaskPtr task(new db::Insert("mail", obj));
  Pool::Queue(task);
}

std::vector<Message> Get(acl::UserID recipient)
{
  QueryResults results;
  mongo::Query query = QUERY("recipient" << recipient);
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("mail", query, results, future));
  Pool::Queue(task);

  future.wait();

  std::vector<Message> mail;
  
  for (auto& obj: results)
    mail.push_back(bson::Message::Unserialize(obj));
    
  return mail;
}

} /* mail namespace */
} /* db namespace */
