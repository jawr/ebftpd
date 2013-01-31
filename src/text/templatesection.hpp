#ifndef __TEXT_TEMPLATESECTION_HPP
#define __TEXT_TEMPLATESECTION_HPP

#include <string>
#include <unordered_map>

#include "text/tag.hpp"
#include "util/error.hpp"

namespace text
{

class TemplateSection
{
  std::string buffer;
  std::vector<Tag> tags;

public:
  void RegisterBuffer(const std::string& buffer) { this->buffer = buffer; }
  std::string RegisterTag(std::string tagStr);

  void RegisterValue(std::string tagName, const std::string& value);
  void RegisterValue(std::string tagName, int value);

  void RegisterSize(std::string tagName, long long kBytes);
  void RegisterSpeed(std::string tagName, long long bytes, long long xfertime);
  void RegisterSpeed(std::string tagName, double speed);

  std::string Compile();
};

// end
}
#endif
