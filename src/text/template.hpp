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
  std::vector<Tag> tags;
  std::vector<std::string> values;

  void CheckValueExists(const std::string& key);
public:
  Template(const std::string& file);
  void RegisterTag(std::string var);

  void RegisterValue(const std::string& key, const std::string& value);
  void RegisterSize(const std::string& key, long long bytes);
  void RegisterSpeed(const std::string& key, long long bytes, long long xfertime);

  std::string Compile();
};

// end
}
#endif

