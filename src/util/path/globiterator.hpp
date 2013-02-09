#ifndef __UTIL_PATH_GLOBITERATOR_HPP
#define __UTIL_PATH_GLOBITERATOR_HPP

#include <fnmatch.h>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <string>
#include <utility>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include "util/string.hpp"
#include "util/path/diriterator.hpp"
#include "util/path/recursivediriterator.hpp"
#include "util/path/status.hpp"
#include "util/path/path.hpp"
#include "util/verify.hpp"
#include "util/enumbitwise.hpp"

namespace util { namespace path
{

class GlobIterator :
  public std::iterator<std::forward_iterator_tag, std::string>
{
public:
  enum Flags { NoFlags = 0, IgnoreErrors = 1 << 0, Recursive = 1 << 1 };
  
private:
  typedef std::vector<std::string> Tokens;

  constexpr static const char* wildcards = "*?[]";
  
  static bool IsWildcard(const std::string& pathToken)
  {
    return pathToken.find_first_of(wildcards) != std::string::npos;
  }
  
  class SubIterator :
    public std::iterator<std::forward_iterator_tag, std::string>
  {
    bool lastToken;
    DirIterator end;
    std::string path;
    Tokens::const_iterator mask;
    Tokens::const_iterator endTokens;
    Flags flags;
    std::shared_ptr<DirIterator> iter;
    
    std::shared_ptr<SubIterator> subIter;
    std::shared_ptr<SubIterator> subEnd;
  
    static bool Filter(const std::string& path, const std::string& mask, bool lastToken)
    {
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

    void NextSub()
    {
      assert(*iter != end);
      auto next = std::find_if(mask + 1, endTokens, IsWildcard);
      if (next == endTokens) --next;
      std::string nextPath(util::path::Join(path, **iter));
      nextPath = util::path::Join(nextPath, util::string::Join(mask + 1, next, "/"));
      try
      {
        subIter.reset(new SubIterator(nextPath, next, endTokens, flags));
        if (!subEnd) subEnd.reset(new SubIterator());
      }
      catch (const util::SystemError& e)
      {
        if (e.Errno() != ENOENT && (flags & IgnoreErrors) == 0) throw;
        else Next();
      }
    }
    
    void First()
    {
      if (!lastToken && *iter != end) NextSub();
    }
    
    void Next()
    {
      if (!lastToken)
      {
        if (subIter)
        {
          if (++(*subIter) != *subEnd)
            return;
          subIter = nullptr;
        }
        
        if (++(*iter) != end) NextSub();
      }
      else
      {
        ++(*iter);
      }
    }
    
    template <typename... Args>
    DirIterator* CreateIterator(bool recursive, Args... args)
    {
      return recursive
             ? new RecursiveDirIterator(std::forward<Args>(args)...) 
             : new DirIterator(std::forward<Args>(args)...);
    }
    
    DirIterator* CreateIterator(bool recursive)
    {
      return CreateIterator(recursive, path, boost::bind(&SubIterator::Filter, _1, 
                    util::path::Join(path, *mask), lastToken), false);
    }
    
  public:
    SubIterator() : lastToken(true), iter(new DirIterator()) { }
    SubIterator(const std::string& path, 
                Tokens::const_iterator mask, 
                Tokens::const_iterator endTokens,
                Flags flags) :
      lastToken(mask == endTokens - 1),
      path(path),
      mask(mask),
      endTokens(endTokens),
      flags(flags),
      iter(CreateIterator(lastToken && flags & Recursive))
    {
      First();
    }
    
    SubIterator& operator++()
    {
      Next();
      return *this;
    }
    
    bool operator==(const SubIterator& rhs)
    { return *iter == *rhs.iter; }
    
    bool operator!=(const SubIterator& rhs)
    { return !operator==(rhs); }
    
    const std::string& operator*() const { return lastToken ? **iter : **subIter; }
    const std::string* operator->() const { return lastToken ? &(**iter) : &(**subIter); }
  };

  Flags flags;
  Tokens pathTokens;
  boost::optional<SubIterator> iter;
  SubIterator end;
  
public:
  GlobIterator() :
    iter(SubIterator())
  {
  }
  
  explicit GlobIterator(std::string pathMask, Flags flags = NoFlags) :
    flags(flags)
  {
    verify(!pathMask.empty());
    
    bool trailingSlash = pathMask.back() == '/';
    boost::split(pathTokens, pathMask, boost::is_any_of("/"),  boost::token_compress_on);
    if (trailingSlash)
    {
      pathTokens.pop_back();
      pathTokens.back() += '/';
    }
    
    auto it = std::find_if(pathTokens.begin(), pathTokens.end(), IsWildcard);
    if (it == pathTokens.end()) --it;

    std::string nextPath = util::string::Join(pathTokens.begin(), it, "/");
    if (nextPath.empty()) nextPath = "/";
    try
    {
      iter.reset(SubIterator(nextPath, it, pathTokens.end(), flags));    
    }
    catch (const util::SystemError& e)
    {
      if ((flags & IgnoreErrors) == 0) throw;
    }
  }
  
  virtual ~GlobIterator() { }

  virtual bool operator==(const GlobIterator& rhs)
  { return *iter == *rhs.iter; }
  
  virtual bool operator!=(const GlobIterator& rhs)
  { return !operator==(rhs); }

  GlobIterator& operator++()
  {
    ++(*iter);
    return *this;
  }
  
  const std::string& operator*() const { return **iter; }
  const std::string* operator->() const { return &(**iter); }
};

} /* path namespace */
} /* util namespace */

#endif
