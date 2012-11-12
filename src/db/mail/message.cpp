#include <cassert>
#include <algorithm>
#include <stdexcept>
#include "db/mail/message.hpp"

namespace db { namespace mail
{

template <> const char* util::EnumStrings<Status>::values[] =
{
  "unread",
  "trash",
  "saved",
  ""
};

} /* mail namespace */
} /* db namespace */
