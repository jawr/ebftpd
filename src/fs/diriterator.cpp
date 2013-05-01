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
#include <boost/bind.hpp>
#include "fs/diriterator.hpp"
#include "acl/path.hpp"
#include "util/path/status.hpp"
#include "fs/iteratorutil.hpp"

namespace fs
{

DirIterator::DirIterator(const acl::User& user, const VirtualPath& path,
                         ValueType valueType) :
  util::path::DirIterator(PreFilter(user, path.ToString()), 
                          [&](const std::string& p)
                          {
                            return Filter(user, (fs::MakeReal(path) / p).ToString());
                          }, 
                          valueType)
{
}

} /* fs namespace */
