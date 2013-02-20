#include "db/dbproxy.hpp"
#include "db/user.hpp"
#include "db/group.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"

namespace db
{

template <typename T, typename IDT, typename DBT>
DBProxy<T, IDT, DBT>::DBProxy(T& obj) :
  obj(&obj), db(new DBT(obj)) 
{
}

template <typename T, typename IDT, typename DBT>
DBProxy<T, IDT, DBT>::DBProxy(const DBProxy& other) :
  obj(other.obj),
  db(new DBT(*other.db))
{
}

template <typename T, typename IDT, typename DBT>
DBProxy<T, IDT, DBT>::~DBProxy()
{
}

template <typename T, typename IDT, typename DBT>
DBProxy<T, IDT, DBT>& DBProxy<T, IDT, DBT>::operator=(const DBProxy<T, IDT, DBT>& rhs)
{
  obj = rhs.obj;
  db.reset(new DBT(*rhs.db));
  return *this;
}

template <typename T, typename IDT, typename DBT>
const DBT* DBProxy<T, IDT, DBT>::operator->() const
{
  return db.get();
}

template <typename T, typename IDT, typename DBT>
DBT* DBProxy<T, IDT, DBT>::operator->()
{
  updated(obj->id);
  return db.get();
}

template <typename T, typename IDT, typename DBT>
void DBProxy<T, IDT, DBT>::ConnectUpdatedSlot(const std::function<void(IDT)>& slot)
{
  updated.connect(slot);
}

template class DBProxy<acl::UserData, acl::UserID, db::User>;
template class DBProxy<acl::GroupData, acl::GroupID, db::Group>;

} /* db namespace */
