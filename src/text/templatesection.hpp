#ifndef __TEXT_TEMPLATESECTION_HPP
#define __TEXT_TEMPLATESECTION_HPP

#include <string>
#include <unordered_map>

#include "text/tag.hpp"
#include "util/error.hpp"

namespace text
{

enum class SectionType
{
  Head,
  Body,
  Foot
};

class TemplateSection
{
  std::string buffer;
  std::vector<Tag> tags;
  std::vector<std::string> values;
  SectionType section;

  void CheckValueExists(const std::string& key);
public:
  TemplateSection(SectionType section) : section(section) {}

  void RegisterBuffer(const std::string& buffer) { this->buffer = buffer; }
  void RegisterTag(std::string var);

  void RegisterValue(const std::string& key, const std::string& value);
  void RegisterValue(const std::string& key, int value);

  void RegisterSize(const std::string& key, long long bytes);
  void RegisterSpeed(const std::string& key, long long bytes, long long xfertime);

  void Reset() { values.clear(); }

  SectionType Section() const { return section; } 

  const std::string& Buffer() const { return buffer; }

  std::string Compile();
};

// end
}
#endif
