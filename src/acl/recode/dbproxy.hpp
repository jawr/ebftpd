#ifndef __ACL_DBPROXY_HPP
#define __ACL_DBPROXY_HPP

#include <boost/signals2.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace acl { namespace recode
{

template <typename T, typename IDT, typename DBT>
class DBProxy
{
  T* obj;
  std::unique_ptr<DBT> db;

  static boost::signals2::signal<void(IDT)> updated;
  
public:
  
  DBProxy(T& obj) : obj(&obj), db(new DBT(obj)) { }
  DBProxy(const DBProxy& other) :
    obj(other.obj),
    db(new DBT(*other.db))
  { }

  ~DBProxy() { }

  DBProxy& operator=(const DBProxy& rhs)
  {
    obj = rhs.obj;
    db.reset(new DBT(*rhs.db));
  }
  
  const DBT* operator->() const { return db.get(); }
  DBT* operator->()
  {
    obj->modified = boost::posix_time::microsec_clock::local_time();
    updated(obj->ID());
    return db.get();
  }
  
  static void ConnectUpdatedSlot(const std::function<void(IDT)>& slot)
  {
    updated.connect(slot);
  }
};

template <typename T, typename IDT, typename DBT>
boost::signals2::signal<void(IDT)> DBProxy<T, IDT, DBT>::updated;

} /* recode namespace */
} /* acl namespace */

#endif
