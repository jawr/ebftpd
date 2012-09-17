#ifndef __CFG_EXCEPTION_HPP
#define __CFG_EXCEPTION_HPP
#include <stdexcept>
namespace cfg
{
struct ConfigError : public std::runtime_error
{
  ConfigError() : std::runtime_error("Config error.") {}
  ConfigError(const std::string& message) : std::runtime_error(message) {}
};

struct ConfigFileError : public ConfigError
{
  ConfigFileError() : ConfigError("Unable to open Config file.") {}
  ConfigFileError(const std::string& message) : ConfigError(message) {}
};

struct NoSetting : public ConfigError
{
  NoSetting() : ConfigError("Error parsing setting") {}
  NoSetting(const std::string& message) : ConfigError(message) {}
};
}
#endif
