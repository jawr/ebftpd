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
    std::function<bool(const std::string&)> filter;
    
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
                Tokens::const_iterator endTokens, Flags flags,
                const std::function<bool(const std::string&)>& filter);
    
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
  
protected:

  std::string pathMask;
  Flags flags;
  std::function<bool(const std::string&)> filter;
  std::shared_ptr<SubIterator> iter;
  SubIterator end;
  
  void Initialise();
  
  static Tokens TokenizePathMask(const std::string& pathMask);
  
public:
  GlobIterator();
  GlobIterator(std::string pathMask, Flags flags = NoFlags);  
  GlobIterator(std::string pathMask, 
               const std::function<bool(const std::string&)>& filter, 
               Flags flags = NoFlags);  
  virtual ~GlobIterator() { }

  GlobIterator& Rewind()
  {
    Initialise();
    return *this;
  }
  
  virtual bool operator==(const GlobIterator& rhs)
  { return *iter == *rhs.iter; }
  
  virtual bool operator!=(const GlobIterator& rhs)
  { return !operator==(rhs); }

  virtual GlobIterator& operator++()
  {
    ++(*iter);
    return *this;
  }
  
  virtual const std::string& operator*() const { return **iter; }
  virtual const std::string* operator->() const { return &**iter; }
};

} /* path namespace */
} /* util namespace */

#endif
