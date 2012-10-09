#ifndef __DB_GET_HPP
#define __DB_GET_HPP
#include "db/pool.hpp"
namespace db
{
  void UpdateShared(const std::tr1::shared_ptr<Pool> newShared);
  Pool& Get();
// end
}
#endif
