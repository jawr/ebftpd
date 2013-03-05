#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <unordered_set>
#include <functional>

/*class HookID;

namespace std
{
template <> class hash<HookID>;
}
*/

struct HookID : public boost::uuids::uuid
{
  HookID(const boost::uuids::uuid& uuid) :
    boost::uuids::uuid(uuid)
  {
  }

  HookID(boost::uuids::uuid&& uuid) :
    boost::uuids::uuid(std::move(uuid))
  {
  }
};

namespace std
{
  template <>
  class hash<boost::uuids::uuid> : public unary_function<boost::uuids::uuid, size_t>
  {
    boost::hash<boost::uuids::uuid> hash;
    
  public:
    size_t operator()(const boost::uuids::uuid& id) const
    {
      return hash(id);
    }
  };
}

int main()
{
  std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uid>> ids;

  boost::uuids::random_generator gen;

  ids.insert(gen());
}
