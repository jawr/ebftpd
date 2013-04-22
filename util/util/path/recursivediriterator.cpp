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

#include "util/path/recursivediriterator.hpp"

namespace util { namespace path
{

std::string RecursiveDirIterator::NextEntry()
{
  if (subIt)
  {
    if (++(*subIt) != *subEnd)
    {
      return **subIt;
    }
    subIt = nullptr;
  }
  
  std::string entry = DirIterator::NextEntry();
  if (!entry.empty() && IsDirectory(entry))
  {
    try
    {
      subIt.reset(new RecursiveDirIterator(entry, filter, ignoreErrors));
      if (!subEnd) subEnd.reset(new RecursiveDirIterator());
    }
    catch (const util::SystemError&)
    {
      if (!ignoreErrors) throw;
    }
  }
  
  return entry;
}

} /* path namespace */
} /* util namespace */
