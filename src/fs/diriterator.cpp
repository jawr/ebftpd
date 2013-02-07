#include <cassert>
#include "fs/diriterator.hpp"
#include "acl/path.hpp"
#include "util/status.hpp"

namespace fs
{

util::Error DirIterator::Check(const fs::Path& path)
{
  assert(user);
  try
  {
    util::path::Status status(path.ToString());
    if (status.IsDirectory())
    {
      return acl::path::DirAllowed<acl::path::View>(*user, MakeVirtual(path));
    }
    else
    {
      return acl::path::FileAllowed<acl::path::View>(*user, MakeVirtual(path));
    }
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
}

} /* fs namespace */
