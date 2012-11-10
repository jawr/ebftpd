#include <cassert>
#include <algorithm>
#include <stdexcept>
#include "db/mail/message.hpp"

namespace db { namespace mail
{

//namespace
//{

const std::string statusStrings[] = 
{
  "unread",
  "trash",
  "saved"
};

//}

std::string StatusToString(db::mail::Status status)
{
  unsigned index = static_cast<unsigned>(status);
  assert(index < sizeof(statusStrings) / sizeof(std::string));
  return statusStrings[index];
}

Status StatusFromString(const std::string& status)
{
  auto begin = std::begin(statusStrings);
  auto end = std::end(statusStrings);
  auto it = std::find(begin, end, status);
  if (it == end) throw std::out_of_range("Invalid status string");
  return static_cast<Status>(std::distance(begin, it));
}

} /* mail namespace */
} /* db namespace */
