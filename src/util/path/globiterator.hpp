#ifndef __UTIL_PATH_GLOBITERATOR_HPP
#define __UTIL_PATH_GLOBITERATOR_HPP

#include <iterator>
#include <string>
#include <vector>
#include <memory>

namespace util { namespace path
{

class DirIterator;

class GlobIterator :
  public std::iterator<std::forward_iterator_tag, std::string>
{
public:
  enum Flags { NoFlags = 0, IgnoreErrors = 1 << 0, Recursive = 1 << 1 };
  
private:
  typedef std::vector<std::string> Tokens;
    
  class SubIterator :
    public std::iterator<std::forward_iterator_tag, std::string>
  {
    bool lastToken;
    std::string path;
    Tokens::const_iterator mask;
    Tokens::const_iterator endTokens;
    Flags flags;
    
    std::shared_ptr<DirIterator> iter;
    std::shared_ptr<DirIterator> end;
    std::shared_ptr<SubIterator> subIter;
    std::shared_ptr<SubIterator> subEnd;
  
    void NextSub();
    void First();
    void Next();

    template <typename... Args> 
    DirIterator* BeginIterator(bool recursive, Args... args);
    
    DirIterator* BeginIterator(bool recursive);
    DirIterator* EndIterator(bool recursive);
    
  public:
    SubIterator();
    SubIterator(const std::string& path, Tokens::const_iterator mask, 
                Tokens::const_iterator endTokens, Flags flags);
    
    SubIterator& operator++()
    {
      Next();
      return *this;
    }
    
    bool operator==(const SubIterator& rhs) const;
    bool operator!=(const SubIterator& rhs) const;
    
    const std::string& operator*() const;
    const std::string* operator->() const;
  };

  Flags flags;
  std::shared_ptr<SubIterator> iter;
  SubIterator end;
  
  static Tokens TokenizePathMask(const std::string& pathMask);
  
public:
  GlobIterator();
  GlobIterator(std::string pathMask, Flags flags = NoFlags);  
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
  const std::string* operator->() const { return &**iter; }
};

} /* path namespace */
} /* util namespace */

#endif
