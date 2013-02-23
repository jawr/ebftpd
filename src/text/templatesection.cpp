#include "util/string.hpp"
#include <boost/algorithm/string/replace.hpp>
#include "text/templatesection.hpp"
#include "text/error.hpp"

namespace text
{

std::string TemplateSection::RegisterTag(std::string tagStr)
{
  util::Trim(tagStr);
  util::ToLower(tagStr);

  std::vector<std::string> args;
  util::Split(args, tagStr, "|");
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

void TemplateSection::DoRegisterValue(std::string tagName, 
    const std::function<void(Tag&)>& doRegister)
{
  util::ToLower(tagName);
  for (auto& tag : tags)
  {
    if (tag.Name() == tagName)
      doRegister(tag);
  }
}
  
void TemplateSection::RegisterValue(const std::string& tagName, const std::string& value)
{
  DoRegisterValue(tagName, [&value](Tag& tag) { tag.RegisterValue(value); });
}

void TemplateSection::RegisterValue(const std::string& tagName, int value)
{
  DoRegisterValue(tagName, [value](Tag& tag) { tag.RegisterValue(value); });
}

void TemplateSection::RegisterSize(const std::string& tagName, long long kBytes)
{
  DoRegisterValue(tagName, [kBytes](Tag& tag) { tag.RegisterSize(kBytes); });
}


void TemplateSection::RegisterSpeed(const std::string& tagName, double speed)
{
  DoRegisterValue(tagName, [speed](Tag& tag) { tag.RegisterSpeed(speed); });
}

void TemplateSection::RegisterSpeed(const std::string& tagName, long long kBytes, 
  long long xfertime)
{
  if (xfertime == 0) RegisterSpeed(tagName, kBytes);
  else RegisterSpeed(tagName, kBytes / xfertime / 1.0);
}

std::string TemplateSection::Compile()
{
  std::string compiled = buffer;
  for (auto& tag : tags)
  {
    boost::replace_first(compiled, "{{" + tag.Name() + "}}", tag.Compile());
  }
  return compiled;
}

}

