#ifndef __DB_DBPROXY_HPP
#define __DB_DBPROXY_HPP

#include <boost/signals2.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace db
{

template <typename T, typename IDT, typename DBT>
class DBProxy
{
  T* obj;
  std::unique_ptr<DBT> db;

  static boost::signals2::signal<void(IDT)> updated;
  
public:
  
  DBProxy(T& obj);
  DBProxy(const DBProxy& other);
  ~DBProxy();
  
  DBProxy& operator=(const DBProxy& rhs);  
  const DBT* operator->() const;
  DBT* operator->();
  static void ConnectUpdatedSlot(const std::function<void(IDT)>& slot);
};

template <typename T, typename IDT, typename DBT>
boost::signals2::signal<void(IDT)> DBProxy<T, IDT, DBT>::updated;

} /* db namespace */

#endif
