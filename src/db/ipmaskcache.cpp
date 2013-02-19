#include "db/ipmaskcache.hpp"
#include "util/string.hpp"
#include "db/error.hpp"
#include "db/serialization.hpp"
#include "db/connection.hpp"

namespace db
{

bool IPMaskCache::IdentIPAllowed(const std::string& identAddress)
{
  boost::lock_guard<boost::mutex> lock(mutex);
  return std::find_if(ipMasks.begin(), ipMasks.end(), 
              [&](const std::pair<acl::UserID, std::vector<std::string>>& kv)
              {
                return util::string::WildcardMatch(kv.second, identAddress, true);
              }) != ipMasks.end();
}

bool IPMaskCache::IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(mutex);
  auto it = ipMasks.find(uid);
  if (it == ipMasks.end()) return false;
  return util::string::WildcardMatch(it->second, identAddress, true);
}

std::vector<std::string> IPMaskNoCache::Lookup(acl::UserID uid)
{
  mongo::Query query;
  if (uid != -1) query = QUERY("uid" << uid);
  auto fields = BSON("ip masks" << 1);
  
  std::vector<std::string> ipMasks;
  NoErrorConnection conn;
  auto results = conn.Query("users", query, 0, 0, &fields);
  for (const auto& obj : results)
  {
    try
    {
      auto masks = UnserializeContainer<decltype(ipMasks)>(obj["ip masks"].Array());
      ipMasks.insert(ipMasks.end(), masks.begin(), masks.end());
    }
    catch (const mongo::DBException& e)
    {
      LogException("Unserialize ip masks", e, "users", query, fields);
    }
  }
  return ipMasks;
}

bool IPMaskNoCache::IdentIPAllowed(const std::string& identAddress)
{
  return util::string::WildcardMatch(Lookup(), identAddress, true);
}

bool IPMaskNoCache::IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  return util::string::WildcardMatch(Lookup(uid), identAddress, true);
}

} /* db namespace */
