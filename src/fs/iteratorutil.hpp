#ifndef __FS_ITERATOR_UTIL_HPP
#define __FS_ITERATOR_UTIL_HPP

namespace fs
{

inline bool Filter(const acl::User& user, const std::string& path)
{
  return acl::path::Allowed<acl::path::View>(user, MakeVirtual(fs::RealPath(path)));
}

inline std::string PreFilter(const acl::User& user, const std::string& path)
{
  RealPath real = MakeReal(fs::Path(path));
  if (!Filter(user, real.ToString())) throw util::SystemError(ENOENT);
  return real.ToString();
}

} /* fs namespace */

#endif
