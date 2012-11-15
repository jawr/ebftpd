#include <boost/algorithm/string.hpp>
#include "text/templatesection.hpp"
#include "text/error.hpp"
#include "text/tag.hpp"

namespace text
{

void TemplateSection::RegisterTag(std::string var)
{
  boost::trim(var);
  boost::to_lower(var);

  std::vector<std::string> args;
  boost::split(args, var, boost::is_any_of("|"));

  std::string name = args.front();
  args.erase(args.begin()); 
  Tag tag(name);;

  while (!args.empty())
  {
    auto filter = args.back();
    args.pop_back();
    
    tag.Register(filter);
  }

  tag.Compile();

  tags.emplace_back(tag);
}

void TemplateSection::CheckValueExists(const std::string& key)
{
  int i = 0;
  for (auto& value: values)
    if (key == value) ++i;
  if (i > 0)
    throw TemplateDuplicateValue("Already registered " + key);
}
  
void TemplateSection::RegisterValue(const std::string& key, const std::string& value)
{
  CheckValueExists(key);
  
  
  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.Parse(value);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

void TemplateSection::RegisterValue(const std::string& key, int value)
{
  CheckValueExists(key);

  std::ostringstream os;
  os << value;

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.Parse(os.str());
  }
  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}


void TemplateSection::RegisterSize(const std::string& key, long long bytes)
{
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.ParseSize(bytes);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

void TemplateSection::RegisterSpeed(const std::string& key, long long bytes, 
  long long xfertime)
{
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.ParseSpeed(bytes, xfertime);
  }

  if (!ok) throw TemplateNoTag("No template tag with key: " + key);
  values.emplace_back(key);
}

std::string TemplateSection::Compile()
{
  std::string ret = buffer;
  for (auto& tag: tags)
  {
    boost::replace_first(ret, "{{" + tag.Name() + "}}", tag.Value());
  }
  return ret;
}

}

