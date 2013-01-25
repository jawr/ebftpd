#ifndef __ACL_REPLICABLE_HPP
#define __ACL_REPLICABLE_HPP

namespace acl
{

class Replicable
{
public:
  virtual ~Replicable() { }
  virtual bool Replicate() = 0;
};

} /* acl namespace */

#endif