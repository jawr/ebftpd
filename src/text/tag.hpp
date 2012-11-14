#ifndef __TEXT_TAG_HPP
#define __TEXT_TAG_HPP

#include <string>
#include <vector>

namespace text
{

enum class TagType
{
  String,
  Float,
  Decimal
};

enum class Alignment
{
  Left,
  Right,
  Center
};

enum class Measurement
{
  Kbyte,
  Mbyte,
  Gbyte,
  Auto,
  None
};

class Tag
{
  std::string name;
  std::string value;
  std::string width;
  std::string precision;
  Alignment alignment;
  Measurement measurement;
  std::string format;
  TagType type;
  bool pretty;
public:
  // oportuinity to set defaults from cofig here
  Tag(const std::string& name) : 
    name(name),
    value("{{" + name + "}}"),
    alignment(Alignment::Right), 
    measurement(Measurement::None), 
    type(TagType::String),
    pretty(false)
   {}
  ~Tag() {}
  template <typename T> std::string Format(T value);
  void Register(const std::string& filter);
  void Compile();
  
  void Parse(const std::string& value);
  void ParseSize(long long bytes);
  void ParseSpeed(long long bytes, long long xfertime);

  const std::string& Name() const { return name; }
  const std::string& Format() const { return format; }
  const std::string& Value() const { return value; }
  const Measurement& Unit() const { return measurement; } // rename all to unit?
 
  void SetType(TagType type) { this->type = type; }
};

// end
}
#endif
