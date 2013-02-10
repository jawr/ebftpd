#include <cassert>
#include <boost/bind.hpp>
#include "fs/globiterator.hpp"
#include "acl/path.hpp"
#include "util/path/status.hpp"
#include "fs/iteratorutil.hpp"

namespace fs
{

GlobIterator::GlobIterator(const acl::User& user, const VirtualPath& path, Flags flags) : 
  util::path::GlobIterator(PreFilter(user, path.ToString()), boost::bind(&Filter, boost::ref(user), _1), flags), 
  user(&user)
{ }

} /* fs namespace */
