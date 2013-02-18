#include <algorithm>
#include <fnmatch.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/bind.hpp>
#include "util/path/globiterator.hpp"
#include "util/path/diriterator.hpp"
#include "util/path/recursivediriterator.hpp"
#include "util/enumbitwise.hpp"
#include "util/string.hpp"
#include "util/path/status.hpp"
#include "util/path/path.hpp"
#include "util/verify.hpp"

namespace util { namespace path
{

namespace
{

bool Filter(const std::string& path, const std::string& mask, bool lastToken,
            const std::function<bool(const std::string&)>& filter)
{
  if (filter && !filter(path)) return false;
  if (!IsDirectory(path))
  {
    if (!lastToken) return false;
    return !fnmatch(mask.c_str(), path.c_str(), 0);
  }
  else
  if (mask.back() == '/')
    return !fnmatch(mask.c_str(), (path + "/").c_str(), FNM_PATHNAME);
  else
    return !fnmatch(mask.c_str(), path.c_str(), 0);
}

bool IsWildcard(const std::string& pathToken)
{
  static const char* wildcards = "*?[]";
  return pathToken.find_first_of(wildcards) != std::string::npos;
}

}

GlobIterator::GlobIterator() :
  iter(new SubIterator())
{
}

GlobIterator::GlobIterator(std::string pathMask, Flags flags) :
  pathMask(pathMask),
	flags(flags)
{
  Initialise();
}

GlobIterator::GlobIterator(std::string pathMask, 
               const std::function<bool(const std::string&)>& filter, 
               Flags flags) :
  pathMask(pathMask),
	flags(flags),
  filter(filter)
{
  Initialise();
}

void GlobIterator::Initialise()
{
	verify(!pathMask.empty());
  
	try
	{
    Tokens toks = TokenizePathMask(pathMask);

    auto next = std::find_if(toks.begin(), toks.end(), IsWildcard);
    if (next == toks.end()) --next;

    std::string nextPath = util::string::Join(toks.begin(), next, "/");
    if (nextPath.empty()) nextPath = "/"; 

		iter.reset(new SubIterator(nextPath, next, toks.end(), flags, filter));    
	}
	catch (const util::SystemError& e)
	{
		if ((flags & IgnoreErrors) == 0) throw;
	}
}

GlobIterator::SubIterator::SubIterator() : 
  lastToken(true), iter(new DirIterator())
{
}

GlobIterator::SubIterator::SubIterator(const std::string& path, 
    Tokens::const_iterator mask, Tokens::const_iterator endTokens, 
    Flags flags, const std::function<bool(const std::string&)>& filter) :
  lastToken(mask == endTokens - 1),
  path(path),
  mask(mask),
  endTokens(endTokens),
  flags(flags),
  filter(filter),
  iter(BeginIterator(lastToken && flags & Recursive)),
  end(EndIterator(lastToken && flags & Recursive))
{
  First();
}

GlobIterator::Tokens GlobIterator::TokenizePathMask(const std::string& pathMask)
{
  bool trailingSlash = pathMask.back() == '/';
  Tokens pathTokens;
  boost::split(pathTokens, pathMask, boost::is_any_of("/"),  boost::token_compress_on);
  if (trailingSlash)
  {
    pathTokens.pop_back();
    pathTokens.back() += '/';  // preserve trailing slash
  }
  return pathTokens;
}

void GlobIterator::SubIterator::NextSub()
{
  assert(*iter != *end);
  auto next = std::find_if(mask + 1, endTokens, IsWildcard);
  if (next == endTokens) --next;
  std::string nextPath(util::path::Join(path, **iter));
  nextPath = util::path::Join(nextPath, util::string::Join(mask + 1, next, "/"));
  
  try
  {
    subIter.reset(new SubIterator(nextPath, next, endTokens, flags, filter));
    if (!subEnd) subEnd.reset(new SubIterator());
  }
  catch (const util::SystemError& e)
  {
    if (e.Errno() != ENOENT && (flags & IgnoreErrors) == 0) throw;
    else Next();
  }
}

void GlobIterator::SubIterator::First()
{
  if (!lastToken && *iter != *end) NextSub();
}

void GlobIterator::SubIterator::Next()
{
  if (!lastToken)
  {
    if (subIter)
    {
      if (++(*subIter) != *subEnd)
        return;
      subIter = nullptr;
    }
    
    if (++(*iter) != *end) NextSub();
  }
  else
  {
    ++(*iter);
  }
}

template <typename... Args>
DirIterator* GlobIterator::SubIterator::BeginIterator(bool recursive, Args&&... args)
{
  return recursive
          ? new RecursiveDirIterator(std::forward<Args>(args)...) 
          : new DirIterator(std::forward<Args>(args)...);
}

DirIterator* GlobIterator::SubIterator::BeginIterator(bool recursive)
{
  return BeginIterator(recursive, path, boost::bind(&Filter, _1, 
                util::path::Join(path, *mask), lastToken, filter), false);
}

DirIterator* GlobIterator::SubIterator::EndIterator(bool recursive)
{
  return recursive
          ? new RecursiveDirIterator()
          : new DirIterator();
}

bool GlobIterator::SubIterator::operator==(const SubIterator& rhs) const
{ return **iter == **rhs.iter; }

bool GlobIterator::SubIterator::operator!=(const SubIterator& rhs) const
{ return !operator==(rhs); }

const std::string& GlobIterator::SubIterator::operator*() const
{ return lastToken ? **iter : **subIter; }

const std::string* GlobIterator::SubIterator::operator->() const
{ return lastToken ? &**iter : &**subIter; }


} /* path namespace */
} /* util namespace */
