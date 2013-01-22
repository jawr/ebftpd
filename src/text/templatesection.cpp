#include <boost/algorithm/string.hpp>
#include "text/templatesection.hpp"
#include "text/error.hpp"
#include "text/tag.hpp"

namespace text
{

std::string TemplateSection::RegisterTag(std::string var)
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

  return name;
}

void TemplateSection::CheckValueExists(const std::string& key)
{
  int i = 0;
  for (auto& value: values)
    if (key == value) ++i;
  if (i > 0)
    throw TemplateDuplicateValue("Already registered " + key);
}
  
void TemplateSection::RegisterValue(std::string key, const std::string& value)
{
  boost::to_lower(key);
  CheckValueExists(key);
  
  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.Parse(value);
  }

  if (ok) //throw TemplateNoTag("No template tag with key: " + key);
    values.emplace_back(key);
}

void TemplateSection::RegisterValue(std::string key, int value)
{
  boost::to_lower(key);
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
  
  if (ok) //throw TemplateNoTag("No template tag with key: " + key);
    values.emplace_back(key);
}


void TemplateSection::RegisterSize(std::string key, long long bytes)
{
  boost::to_lower(key);
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.ParseSize(bytes);
  }

  if (ok) //throw TemplateNoTag("No template tag with key: " + key);
    values.emplace_back(key);
}


void TemplateSection::RegisterSpeed(std::string key, double speed)
{
  boost::to_lower(key);
  CheckValueExists(key);

  bool ok = false;
  for (auto& tag: tags)
  {
    if (tag.Name() != key) continue;
    ok = true;
    tag.ParseSpeed(speed);
  }

  if (ok) //throw TemplateNoTag("No template tag with key: " + key);
    values.emplace_back(key);
}

void TemplateSection::RegisterSpeed(std::string key, long long bytes, 
  long long xfertime)
{
  if (xfertime == 0) RegisterSpeed(key, bytes);
  else RegisterSpeed(key, bytes / xfertime / 1.0);
}
std::string TemplateSection::Compile()
{
  std::string ret = buffer;
  for (auto& tag: tags)
  {
    boost::replace_first(ret, "{{" + tag.Name() + "}}", tag.Value());
  }
  // replace tags that haven't been registered with blank values
  for (auto& tag: tags)
  {
    boost::replace_first(ret, "{{" + tag.Name() + "}}", "");
  }
  return ret;
}

}

