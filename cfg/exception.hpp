#ifndef __CFG_EXCEPTION_HPP
#define __CFG_EXCEPTION_HPP

#include "util/error.hpp"

namespace cfg
{

struct ConfigError : public util::RuntimeError
{
  ConfigError() : std::runtime_error("Config error.") {}
  ConfigError(const std::string& message) : std::runtime_error(message) {}
};

struct ConfigFileError : public ConfigError
{
  ConfigFileError() : std::runtime_error("Unable to open Config file.") {}
  ConfigFileError(const std::string& message) : std::runtime_error(message) {}
};

struct NoSetting : public ConfigError
{
  NoSetting() : std::runtime_error("Error parsing setting") {}
  NoSetting(const std::string& message) : std::runtime_error(message) {}
};

struct RequiredSetting : public ConfigError
{
  RequiredSetting() : std::runtime_error("Missing required setting.") {}
  RequiredSetting(const std::string& message) : std::runtime_error("Missing required setting: " + message) {}
};

}

#endif
