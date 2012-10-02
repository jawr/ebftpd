#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP

#include <string>
#include <vector>

namespace cfg { namespace setting
{
class Setting {
public:
  virtual void Save() = 0;
};

class AsciiDownloads : public Setting
{
  int size;
  std::vector<string> masks;
public:
  AsciiDownloads() {};
  virtual void Save(const std::vector<std::string>& toks);
};

class Shutdown : public Setting
{
  acl::ACL;
public:
  Shutdown() {};
  virtual void Save(const std::vector<std::string>& toks);
};

class FreeSpace : public Setting
{
  int amount;
public:
  FreeSpace() {}
  virtual void Save(const std::vector<std::string>& toks);
};

class UseDirSize : public Setting
{
  char unit;
  std::vector<fs::Path> paths;
public:
  UseDirSize() {};
  virtual void Save(const std::vector<std::string>& toks);
};

class Timezone : public Setting
{
  unsigned int hours;
public:
  Timezone() {};
  virtual void Save(const std::vector<std::string>& toks);
};

class ColorMode : public Setting
{
  bool use;
public:
  ColorMode() {};
  virtual void Save(const std::vector<std::string>& toks);
};

class SitenameLong : public Setting
{
  std::string name;
public:
  SitenameLong() {};
  virtual void Save(const std::vector<std::string>& toks);
};

class SitenameShort : public Setting
{
  std::string name;
public:
  SitenameShort() {};
  virtual void Save(const std::vector<std::string>& toks);
};

// end namespace
}
}
#endif
