#include <cassert>
#include <boost/bind.hpp>
#include "fs/globiterator.hpp"
#include "acl/path.hpp"
#include "util/path/status.hpp"

namespace fs
{

namespace
{

bool Filter(const acl::User& user, const std::string& path)
{
  return acl::path::Allowed<acl::path::View>(user, MakeVirtual(fs::RealPath(path)));
}

std::string PreFilter(const acl::User& user, const std::string& path)
{
  RealPath real = MakeReal(fs::Path(path));
  if (!Filter(user, real.ToString())) throw util::SystemError(ENOENT);
  return real.ToString();
}

}

GlobIterator::GlobIterator(const acl::User& user, const VirtualPath& path, Flags flags) : 
  util::path::GlobIterator(PreFilter(user, path.ToString()), boost::bind(&Filter, boost::ref(user), _1), flags), 
  user(&user)
{ }

} /* fs namespace */
