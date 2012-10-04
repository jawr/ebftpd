#include "util/string.hpp"
namespace util { namespace string
{
bool BoolLexicalCast(const std::string& arg)
{
  std::string arg_ = arg;
  boost::algorithm::to_lower(arg_);
  if (arg_ == "yes") return true;
  else if (arg_ == "no") return false;
  return boost::lexical_cast<bool>(arg);
}
}
}

