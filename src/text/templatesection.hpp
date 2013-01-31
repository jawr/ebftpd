#ifndef __TEXT_TEMPLATESECTION_HPP
#define __TEXT_TEMPLATESECTION_HPP

#include <string>
#include <unordered_map>
#include <functional>
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
  
  void DuplicateTags(TemplateSection& section) const 
  { section.tags.insert(section.tags.end(), tags.begin(), tags.end()); }
  
  bool HasTag(const std::string& tagName) const
  {
    return std::find_if(tags.begin(), tags.end(), 
                  [tagName](const Tag& tag) { return tag.Name() == tagName; }) == tags.end();
  }
};

// end
}
#endif
