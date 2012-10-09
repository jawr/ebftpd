#ifndef __DB_EXCEPTION_HPP
#define __DB_EXCEPTION_HPP
namespace db
{
struct DBError : public std::runtime_error
{
  DBError() : std::runtime_error("DB error.") {}
  DBError(const std::string& message) : std::runtime_error(message) {}
};
}
#endif
