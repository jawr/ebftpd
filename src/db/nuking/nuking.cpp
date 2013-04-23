#include <boost/optional.hpp>
#include "db/nuking/nuking.hpp"
#include "db/serialization.hpp"
#include "db/connection.hpp"

namespace db
{

template <> mongo::BSONObj Serialize<nuking::Nukee>(const nuking::Nukee& nukee)
{
  mongo::BSONObjBuilder bob;
  bob.append("uid", nukee.UID());
  bob.append("kbytes", nukee.KBytes());
  bob.append("files", nukee.Files());
  bob.append("credits", nukee.Credits());
  return bob.obj();
}

template <> nuking::Nukee Unserialize<nuking::Nukee>(const mongo::BSONObj& obj)
{
  return nuking::Nukee(obj["uid"].Int(), 
                       obj["kbytes"].Long(), 
                       obj["files"].Int(),
                       obj["credits"].Long());
}

template <> mongo::BSONObj Serialize<nuking::Nuke>(const nuking::Nuke& nuke)
{
  mongo::BSONObjBuilder bob;
  bob.append("path", nuke.Path());
  bob.append("section", nuke.Section());
  bob.append("multiplier", nuke.Multiplier());
  bob.append("percent", nuke.IsPercent());
  bob.append("modtime", ToDateT(nuke.ModTime() * 1000));
  
  mongo::BSONArrayBuilder bab;
  for (const auto& nukee : nuke.Nukees())
    bab.append(Serialize(nukee));    
  bob.append("nukees", bab.arr());
  
  return bob.obj();
}

template <> nuking::Nuke Unserialize<nuking::Nuke>(const mongo::BSONObj& obj)
{
  std::vector<nuking::Nukee> nukees;  
  for (auto& elem : obj["nukees"].Array())
    nukees.emplace_back(Unserialize<nuking::Nukee>(elem.Obj()));
  
  mongo::BSONElement oid;
  obj.getObjectID(oid);

  return nuking::Nuke(oid.OID().toString(),
                      obj["path"].String(),
                      obj["section"].String(),
                      obj["multiplier"].Int(),
                      obj["percent"].Bool(),
                      obj["modtime"].Date().toTimeT(),
                      ToPosixTime(oid.OID().asDateT()),
                      nukees);
}

namespace nuking
{

void AddNuke(const Nuke& nuke)
{
  NoErrorConnection conn;
  conn.InsertOne("nukes", nuke);
}

bool DelNuke(const Nuke& nuke)
{
  NoErrorConnection conn;
  return conn.Remove("nukes", QUERY("path" << nuke.Path())) > 0;
}

boost::optional<Nuke> LookupNuke(const std::string& id)
{
  NoErrorConnection conn;
  return conn.QueryOne<Nuke>("nukes", QUERY("_id" << mongo::OID(id)));
}

void AddUnnuke(const Nuke& nuke)
{
  NoErrorConnection conn;
  conn.InsertOne("nukees", nuke);
}

bool DelUnnuke(const Nuke& nuke)
{
  NoErrorConnection conn;
  return conn.Remove("unnukes", QUERY("path" << nuke.Path())) > 0;
}

std::vector<Nuke> Newest(const std::string& collection, int limit)
{
  NoErrorConnection conn;
  return conn.QueryMulti<Nuke>(collection, mongo::Query().sort("_id", -1), limit);
}

std::vector<Nuke> NewestNukes(int limit)
{
  return Newest("nukes", limit);
}

std::vector<Nuke> NewestUnnukes(int limit)
{
  return Newest("unnukes", limit);
}

} /* nuking namespace */
} /* db namespace */
