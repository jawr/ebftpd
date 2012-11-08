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
  std::string width;
  std::string precision;
  Alignment alignment;
  Measurement measurement;
  std::string format;
  TagType type;
public:
  // oportuinity to set defaults from cofig here
  Tag() : alignment(Alignment::Right), measurement(Measurement::None), 
    type(TagType::String) {}
  ~Tag() {}
  template <typename T> std::string Format(T value);
  void Register(const std::string& filter);
  void Compile();
  const std::string& Format() const { return format; }
  
  void SetType(TagType type) { this->type = type; }
};

// end
}
#endif
