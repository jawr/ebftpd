#include "db/mail/mail.hpp"
#include "db/mail/message.hpp"
#include "db/bson/message.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"

namespace db
{

typedef std::shared_ptr<Task> TaskPtr;
typedef std::vector<mongo::BSONObj> QueryResults;

namespace mail
{

void Send(const Message& message)
{
  mongo::BSONObj obj = db::bson::Message::Serialize(message);
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
    mail.emplace_back(bson::Message::Unserialize(obj));
    
  return mail;
}

bool Save(acl::UserID recipient, int index)
{
  QueryResults results;

  {
    mongo::Query query = QUERY("recipient" << recipient);
    boost::unique_future<bool> future;
    TaskPtr task(new db::Select("mail", query, results, future, 1, index));
    Pool::Queue(task);
    future.wait();
    
    if (results.empty()) return false;
  }
  
  mongo::BSONElement oid;
  results.front().getObjectID(oid);
  mongo::Query query = QUERY("_id" << oid.OID());
  boost::unique_future<int> future;
  TaskPtr task(new db::Update("mail", query, BSON("$set" << BSON("status" << "saved")), future));
  Pool::Queue(task);
  
  future.wait();
  return future.get() > 0;
}

unsigned SaveTrash(acl::UserID recipient)
{
  mongo::Query query = QUERY("recipient" << recipient << "status" << "trash");
  boost::unique_future<int> future;
  TaskPtr task(new db::Update("mail", query, BSON("$set" << BSON("status" << "saved")), future));
  Pool::Queue(task);
  future.wait();
  int purged = future.get();
  return purged == -1 ? 0 : purged;
}

bool Purge(acl::UserID recipient, int index)
{
  QueryResults results;
  
  {
    mongo::Query query = QUERY("recipient" << recipient);
    boost::unique_future<bool> future;
    TaskPtr task(new db::Select("mail", query, results, future, 1, index));
    Pool::Queue(task);
    future.wait();
    
    if (results.empty()) return false;
  }
  
  mongo::BSONElement oid;
  results.front().getObjectID(oid);
  mongo::Query query = QUERY("_id" << oid.OID());
  boost::unique_future<int> future;
  TaskPtr task(new db::Delete("mail", query, future));
  Pool::Queue(task);
  future.wait();
  
  return future.get() > 0;
}

unsigned PurgeTrash(acl::UserID recipient)
{
  mongo::Query query = QUERY("recipient" << recipient << "status" << "trash");
  boost::unique_future<int> future;
  TaskPtr task(new db::Delete("mail", query, future));
  Pool::Queue(task);
  future.wait();
  int purged = future.get();
  return purged == -1 ? 0 : purged;
}

void LogOffPurgeTrash(acl::UserID recipient)
{
  mongo::Query query = QUERY("recipient" << recipient << "status" << "trash");
  TaskPtr task(new db::Delete("mail", query));
  Pool::Queue(task);
}

void Trash(const Message& message)
{
  mongo::Query query = QUERY("_id" << message.oid);
  TaskPtr task(new db::Update("mail", query, BSON("$set" << BSON("status" << "trash"))));
  Pool::Queue(task);
}

} /* mail namespace */
} /* db namespace */
