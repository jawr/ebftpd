#include "util/format.hpp"

#ifdef __UTIL_FORMAT_TEST

enum class String
{
  UserNotExist,
  GroupNotExist,
  UnableCreateUser
};

template <> const char* util::EnumStrings<String>::values[]
{
  "User %1% doesn't exist.",
  "Group %1% doesn't exist.",
  "Unable to create user %1%: %2%"
};

int main()
{
  {
    util::Format Format([](const std::string& s) { std::cout << s << std::endl; });
   
    Format(String::UserNotExist, "someone", "ignores", "too many", "args");
    Format(String::GroupNotExist, "SOMEGROUP");
    Format(String::UnableCreateUser, "someone", "some error message");
  }
  
  {
    util::Format Format;
    std::cout << Format("hello %s%c", "world", '!') << std::endl;
    
    std::string s = Format("some %s string %i", "other", 123).String();
    std::cout << s << std::endl;
    
    s = Format("muahahaha %.2f", 23.676457);
    std::cout << s << std::endl;
  }
}

#endif
