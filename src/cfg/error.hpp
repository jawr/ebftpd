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

struct RequiredSetting : public ConfigError
{
  RequiredSetting() : std::runtime_error("Missing required setting.") {}
  RequiredSetting(const std::string& setting) : std::runtime_error("Missing required setting: " + setting) {}
};

}

#endif
