#ifndef __TEXT_TYPE_HPP
#define __TEXT_TYPE_HPP

#include <string>
#include <vector>

namespace text
{

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
  Auto
};

class Type
{
  std::string width;
  std::string precision;
  Alignment alignment;
  Measurement measurement;
  std::string format;
public:
  // oportuinity to set defaults from cofig here
  Type() : alignment(Alignment::Left), measurement(Measurement::Auto) {}
  ~Type() {}
  template <typename T> std::string Format(T value);
  void Register(const std::string& filter);
  void Compile();
  const std::string& Format() const { return format; }
};

// end
}
#endif
