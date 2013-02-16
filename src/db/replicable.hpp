#ifndef __DB_REPLICABLE_HPP
#define __DB_REPLICABLE_HPP

#include <string>

namespace db
{

class Replicable
{
private:
  std::string name;
  
public:
  Replicable(const std::string& name) : name(name) { }
  virtual ~Replicable() { }
  virtual bool Replicate() = 0;
  
  const std::string& Name() const { return name; }
};

} /* db namespace */

#endif