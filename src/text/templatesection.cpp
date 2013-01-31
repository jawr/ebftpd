#include <boost/algorithm/string.hpp>
#include "text/templatesection.hpp"
#include "text/error.hpp"
#include "text/tag.hpp"

namespace text
{

std::string TemplateSection::RegisterTag(std::string tagStr)
{
  boost::trim(tagStr);
  boost::to_lower(tagStr);

  std::vector<std::string> args;
  boost::split(args, tagStr, boost::is_any_of("|"));
  if (args.empty()) throw TemplateError("Invalid tag: " + tagStr);
  
  const std::string& name = args.front();
  Tag tag(name);

  for (auto it = args.begin() + 1; it != args.end(); ++it)
  {
    tag.SetFilter(*it);
  }

  tags.emplace_back(tag);

  return name;
}

/*void TemplateSection::CheckAlreadyRegistered(const std::string& tagName)
{
  int i = 0;
  for (auto& value: values)
    if (key == value) ++i;
  if (i > 0)
    throw TemplateDuplicateValue("Already registered " + tagName);
}*/
  
void TemplateSection::RegisterValue(std::string tagName, const std::string& value)
{
  boost::to_lower(tagName);
  
  auto it = std::find_if(tags.begin(), tags.end(),
    [tagName](const Tag& tag) { return tag.Name() == tagName; });

  if (it != tags.end()) it->RegisterValue(value);
}

void TemplateSection::RegisterValue(std::string tagName, int value)
{
  boost::to_lower(tagName);
  
  auto it = std::find_if(tags.begin(), tags.end(),
    [tagName](const Tag& tag) { return tag.Name() == tagName; });

  if (it != tags.end()) it->RegisterValue(value);
}


void TemplateSection::RegisterSize(std::string tagName, long long kBytes)
{
  boost::to_lower(tagName);
  
  auto it = std::find_if(tags.begin(), tags.end(),
    [tagName](const Tag& tag) { return tag.Name() == tagName; });

  if (it != tags.end()) it->RegisterSize(kBytes);
}


void TemplateSection::RegisterSpeed(std::string tagName, double speed)
{
  boost::to_lower(tagName);
  
  auto it = std::find_if(tags.begin(), tags.end(),
    [tagName](const Tag& tag) { return tag.Name() == tagName; });

  if (it != tags.end()) it->RegisterSpeed(speed);
}

void TemplateSection::RegisterSpeed(std::string tagName, long long kBytes, 
  long long xfertime)
{
  if (xfertime == 0) RegisterSpeed(tagName, kBytes);
  else RegisterSpeed(tagName, kBytes / xfertime / 1.0);
}

std::string TemplateSection::Compile()
{
  std::string compiled = buffer;
  for (auto& tag: tags)
  {
    boost::replace_first(compiled, "{{" + tag.Name() + "}}", tag.Compile());
  }
  return compiled;
}

}

