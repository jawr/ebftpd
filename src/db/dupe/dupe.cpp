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

#include <boost/regex.hpp>
#include <future>
#include "db/dupe/dupe.hpp"
#include "db/connection.hpp"
#include "db/serialization.hpp"
#include "util/misc.hpp"

namespace db
{

template <> dupe::DupeResult Unserialize<dupe::DupeResult>(const mongo::BSONObj& obj)
{
  try
  {
    mongo::BSONElement oid;
    obj.getObjectID(oid);
    return dupe::DupeResult(obj["directory"].String(),
                            obj["section"].String(),
                            db::ToPosixTime(oid.OID().asDateT()));
  }
  catch (const mongo::DBException& e)
  {
    LogException("Dupe result unserialize", e, obj);
    throw e;
  }
}

namespace dupe
{

void Add(const std::string& directory, const std::string& section)
{
  FastConnection conn;
  conn.Insert("dupe", BSON("directory" << directory << 
                           "section" << section <<
                           "nuked" << false));
}

std::vector<DupeResult> Search(const std::vector<std::string>& terms, int limit)
{
  mongo::BSONObjBuilder bob;
  for (const std::string& term : terms)
  {
    bob.appendRegex("directory", util::EscapeRegex(term), "i");
  }
  
  mongo::Query query(bob.obj());
  
  NoErrorConnection conn;
  return conn.QueryMulti<DupeResult>("dupe", query, limit);
}

} /* dupe namespace */
} /* db namespace */
