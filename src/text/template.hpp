#ifndef __TEXT_TEMPLATE_HPP
#define __TEXT_TEMPLATE_HPP

#include <string>
#include <unordered_map>

#include "text/tag.hpp"

namespace text
{

class Template
{
  std::string file;
  std::string buffer;
  std::unordered_map<std::string, Tag> tags;
  std::unordered_map<std::string, std::string> values;
public:
  Template(const std::string& file);
  void RegisterTag(std::string var);
  void RegisterValue(const std::string& key, const std::string& value);
  std::string Compile();
};

// end
}
#endif

