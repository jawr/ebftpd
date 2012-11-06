#ifndef __DB_EXCEPTION_HPP
#define __DB_EXCEPTION_HPP

#include "util/error.hpp"
namespace db
{
struct DBError : public util::RuntimeError
{
  DBError() : std::runtime_error("Unknown DB error.") {}
  DBError(const std::string& message) : std::runtime_error(message) {}
};
}
#endif
