//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cassert>
#include <boost/filesystem.hpp>
#include "util/path/globiterator.hpp"
#include "util/error.hpp"
#include "util/string.hpp"
#include "util/path/path.hpp"

namespace util { namespace path
{

namespace fs = boost::filesystem3;

GlobIterator::GlobIterator() :
  iter(new fs::recursive_directory_iterator()),
  recursive(false),
  trailingSlash(false)
{
}

GlobIterator::GlobIterator(const std::string& pathMask, bool recursive) :
  recursive(recursive),
  trailingSlash(false)
{
  TokenizePath(pathMask);
  OpenDirectory();
}

GlobIterator::GlobIterator(const std::string& pathMask, 
        const std::function<bool(const std::string&)>& filter, bool recursive) :
  filter(filter),
  recursive(recursive),
  trailingSlash(false)
{
  TokenizePath(pathMask);
  OpenDirectory();
}

void GlobIterator::TokenizePath(std::string pathMask)
{
  try
  {
    trailingSlash = pathMask.back() == '/';
    pathMask = Resolve(fs::absolute(pathMask).string());
    util::Split(pathTokens, pathMask, "/", true);
    if (!pathTokens.empty())
    {
      pathTokens.erase(pathTokens.begin());
    }
  }
  catch (const fs::filesystem_error& e)
  {
    throw util::SystemError(e.code().value());
  }
}

void GlobIterator::OpenDirectory()
{
  try
  {
    iter.reset(new fs::recursive_directory_iterator("/"));
    iter->no_push(true);
    Next();
  }
  catch (const fs::filesystem_error& e)
  {
    throw util::SystemError(e.code().value());
  }
}

GlobIterator::~GlobIterator()
{
}

GlobIterator& GlobIterator::Rewind()
{
  OpenDirectory();
  return *this;
}

bool GlobIterator::operator==(const DirIteratorBase& rhs)
{
  assert(dynamic_cast<const GlobIterator*>(&rhs));
  return *this->iter == *reinterpret_cast<const GlobIterator*>(&rhs)->iter;
}

bool GlobIterator::operator!=(const DirIteratorBase& rhs)
{
  return !operator==(rhs);
}

GlobIterator& GlobIterator::operator++()
{
  Next();
  return *this;
}

const std::string& GlobIterator::operator*() const
{
  current = (*iter)->path().string();
  return current;
}

const std::string* GlobIterator::operator->() const
{
  return &operator*();
}

void GlobIterator::Next()
{
  try
  {
    fs::recursive_directory_iterator end;
    while (++(*iter) != end)
    {
      try
      {
        iter->no_push(true);
        auto& path = (*iter)->path();
        if (iter->level() < static_cast<ssize_t>(pathTokens.size()) - 1)
        {
          if (fs::is_directory(path) && WildcardMatch(pathTokens[iter->level()], path.filename().string()))
          {
            iter->no_push(false);
          }
        }
        else if (iter->level() == static_cast<ssize_t>(pathTokens.size()) - 1)
        {
          if (WildcardMatch(pathTokens[iter->level()], path.filename().string()))
          {
            if (fs::is_directory(path))
            {
              if (recursive)
              {
                iter->no_push(false);
              }
            }
            else if (trailingSlash)
            {
              continue;
            }
            current = (*iter)->path().string();
            break;
          }
        }
        else if (iter->level() >= static_cast<ssize_t>(pathTokens.size()))
        {
          assert(recursive);
          if (fs::is_directory(path))
          {
            iter->no_push(false);
          }
          current = (*iter)->path().string();
          break;
        }
      }
      catch (const fs::filesystem_error&)
      {
      }
    }
  }
  catch (const fs::filesystem_error& e)
  {
    throw util::SystemError(e.code().value());
  }
}

} /* path namespace */
} /* util namespace */

