#include "db/mail/message.hpp"

namespace util
{

template <> const char* util::EnumStrings<db::mail::Status>::values[] =
{
  "unread",
  "trash",
  "saved",
  ""
};

}

