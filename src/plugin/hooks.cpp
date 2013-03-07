#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include "plugin/hooks.hpp"

namespace plugin
{

HookID GenerateHookID()
{
  boost::mt19937 ran(time(NULL));
  boost::uuids::random_generator generate(&ran);
  return generate();
}


std::string HookIDToString(const HookID& id)
{
  return boost::uuids::to_string(id);
}

bool HookIDFromString(const std::string& s, HookID& id)
{
  std::istringstream is(s);
  is >> id;
  return !is.bad() && !is.fail();
}

bool IntegerToHookResult(int iresult, HookResult& result)
{
  if (iresult < static_cast<int>(HookResult::First) ||
      iresult > static_cast<int>(HookResult::Last))
  {
    return false;
  }
  
  result = static_cast<HookResult>(iresult);
  return true;
}

boost::optional<CommandHook> CommandHooks::Lookup(const std::string& command)
{
  auto it = hooks.find(command);
  if (it == hooks.end()) return boost::none;
  return boost::make_optional(it->second);
}

boost::optional<HookID> CommandHooks::Connect(
      const std::string& command, const std::string& description, 
      const std::string& syntax, const std::string& acl, 
      const CommandHookFunction& function)
{
  // site factory singleton needs to be fixed
  //if (cmd::site::Factory::Lookup(command)) return boost::none;
  auto hook = CommandHook(function, GenerateHookID(), command, description, syntax, acl);
  auto it = hooks.insert(std::make_pair(command, std::move(hook)));
  if (!it.second) return boost::none;
  return boost::make_optional(it.first->second.id);
}

void CommandHooks::Disconnect(const HookID& id)
{
  auto it = std::find_if(hooks.begin(), hooks.end(),
                [&id](const std::pair<std::string, CommandHook>& kv)
                {
                  return kv.second.id == id;
                });
  if (it != hooks.end())
  {
    hooks.erase(it);
  }
}

void EventHooks::Trigger(Event event, Client& client, const EventHookArgs& args, bool& okay)
{
  auto range = hooks.equal_range(event);
  for (auto it = range.first; it != range.second; ++it)
  {
    if (okay || it->second.always)
    {
      if (it->second.function(client, args) != HookResult::Okay)
      {
        okay = false;
      }
    }
  }
}

HookID EventHooks::Connect(Event event, const EventHookFunction& function, bool always)
{
  auto it = hooks.insert(std::make_pair(event, EventHook(function, GenerateHookID(), always)));
  return it->second.id;
}

void EventHooks::Disconnect(const HookID& id)
{
  auto it = std::find_if(hooks.begin(), hooks.end(),
                [&id](const std::pair<Event, EventHook>& kv)
                {
                  return kv.second.id == id;
                });
  if (it != hooks.end())
  {
    hooks.erase(it);
  }
}

} /* plugin namespace */
