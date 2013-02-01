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

void TemplateSection::DoRegisterValue(std::string tagName, 
    const std::function<void(Tag&)>& doRegister)
{
  boost::to_lower(tagName);
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
  std::cout << tags.size() << std::endl;
  std::string compiled = buffer;
  std::cout << compiled << std::endl;
  for (auto& tag : tags)
  {
    std::cout << tag.Name() << " " << tag.Compile() << std::endl;
    boost::replace_first(compiled, "{{" + tag.Name() + "}}", tag.Compile());
  }
  return compiled;
}

}

