#include <cassert>
#include <boost/bind.hpp>
#include "fs/diriterator.hpp"
#include "acl/path.hpp"
#include "util/path/status.hpp"
#include "fs/iteratorutil.hpp"

namespace fs
{

DirIterator::DirIterator(const acl::User& user, const VirtualPath& path) : 
  util::path::DirIterator(PreFilter(user, path.ToString()), boost::bind(&Filter, boost::ref(user), _1))
  //, user(&user)
{ }

} /* fs namespace */
