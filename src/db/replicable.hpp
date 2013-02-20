#ifndef __DB_REPLICABLE_HPP
#define __DB_REPLICABLE_HPP

#include <string>

namespace mongo
{
class BSONElement;
}

namespace db
{

enum class ReplicationState
{
  Populate,
  Replicate
};

class Replicable
{
private:
  std::string collection;
  ReplicationState state;
  
public:
  Replicable(const std::string& collection) : 
    collection(collection), 
    state(ReplicationState::Populate)
  { }
  
  virtual ~Replicable() { }

  virtual bool Replicate(const mongo::BSONElement& id) = 0;
  virtual bool Populate() = 0;
  
  const std::string& Collection() const { return collection; }
};

} /* db namespace */

#endif