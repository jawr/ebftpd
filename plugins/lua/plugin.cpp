extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luaconf.h>
}

#include <cmath>
#include <iostream>
#include <cassert>
#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/class_info.hpp>
#include <luabind/exception_handler.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <sys/time.h>
#include "lua/plugin.hpp"
#include "plugin/user.hpp"
#include "plugin/group.hpp"
//#include "plugin/mail.hpp"
#include "plugin/hooks.hpp"
#include "plugin/error.hpp"
#include "plugin/client.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "fs/owner.hpp"
#include "db/mail/message.hpp"
#include "db/mail/mail.hpp"
#include "plugin/plugin.hpp"
#include "util/path/path.hpp"
#include "ftp/xdupe.hpp"
#include "logs/logs.hpp"

namespace luabind
{

template <typename T>
struct default_converter<boost::optional<T>> : native_converter_base<boost::optional<T>>
{
  static int compute_score(lua_State* /* L */, int /* index */)
  {
    return 0;
  }

  boost::optional<T> from(lua_State* L, int index)
  {
    object obj(from_stack(L, index));
    if (!obj) return boost::none;
    else return boost::optional<T>(object_cast<T>(obj));
  }
  
  void to(lua_State* L, const boost::optional<T>& opt)
  {
    if (!opt) lua_pushnil(L);
    else object(L, *opt).push(L);
  }
};  

template <typename T>
struct default_converter<const boost::optional<T>&> : default_converter<boost::optional<T>> { };

template <typename T>
struct default_converter<const boost::optional<T>> : default_converter<boost::optional<T>> { };

template <typename T1, typename T2>
struct default_converter<std::pair<T1, T2>> : native_converter_base<std::pair<T1, T2>>
{
  static int compute_score(lua_State* L, int index)
  {
    return 1;
  }

  std::pair<T1, T2> from(lua_State* L, int index)
  {
    verify(false);
  }
  
  void to(lua_State* L, const std::pair<T1, T2>& pair)
  {
		object(L, pair.first).push(L);
		object(L, pair.second).push(L);
  }
};  

template <typename T1, typename T2>
struct default_converter<const std::pair<T1, T2>&> : default_converter<std::pair<T1, T2>> { };

template <typename C>
C to_back_insertable(lua_State* L, int index)
{
  C container;
  typedef typename C::value_type value_type;
  iterator it(object(from_stack(L, index)));
  iterator end;
  for (; it != end; ++it)
    container.push_back(object_cast<value_type>(*it));
  return container;
}

template <typename C>
C to_insertable(lua_State* L, int index)
{
  C container;
  typedef typename C::value_type value_type;
  iterator it(object(from_stack(L, index)));
  iterator end;
  for (; it != end; ++it)
    container.insert(object_cast<value_type>(*it));
  return container;
}

template <typename C>
C to_associative(lua_State* L, int index)
{
  C container;
  typedef typename C::key_type key_type;
  typedef typename C::mapped_type mapped_type;
  iterator it(object(from_stack(L, index)));
  iterator end;
  for (; it != end; ++it)
    container.insert(std::make_pair(object_cast<mapped_type>(it.key), 
                                    object_cast<mapped_type>(*it)));
  return container;
}

template <typename C>
void from_container(lua_State* L, const C& container)
{
  object table = newtable(L);
  int index = 0;
  for (auto& elem : container)
    table[++index] = elem;
  table.push(L);
}

template <typename T>
struct default_converter<std::vector<T>> : native_converter_base<std::vector<T>>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
  }

  std::vector<T> from(lua_State* L, int index)
  {
    return to_back_insertable<std::vector<T>>(L, index);
  }
  
  void to(lua_State* L, const std::vector<T>& container)
  {
    from_container(L, container);
  }
};  

template <typename T>
struct default_converter<const std::vector<T>&> : default_converter<std::vector<T>> { };

template <typename T>
struct default_converter<const std::vector<T>> : default_converter<std::vector<T>> { };

template <typename T>
struct default_converter<std::unordered_set<T>> : native_converter_base<std::unordered_set<T>>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
  }

  std::unordered_set<T> from(lua_State* L, int index)
  {
    return to_insertable<std::unordered_set<T>>(L, index);
  }
  
  void to(lua_State* L, const std::unordered_set<T>& container)
  {
    from_container(L, container);
  }
};  

template <typename T>
struct default_converter<const std::unordered_set<T>&> : default_converter<std::unordered_set<T>> { };

template <>
struct default_converter<boost::gregorian::date> : native_converter_base<boost::gregorian::date>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
  }

  boost::gregorian::date from(lua_State* L, int index)
  {
    time_t t = object_cast<time_t>(object(from_stack(L, index)));
    auto ptime = boost::posix_time::from_time_t(t);
    return ptime.date();
  }
  
  void to(lua_State* L, const boost::gregorian::date& date)
  {
    struct tm tm = boost::gregorian::to_tm(date);
    lua_pushnumber(L, mktime(&tm));
  }
};  

template <>
struct default_converter<const boost::gregorian::date> : default_converter<boost::gregorian::date> { };

template <>
struct default_converter<const boost::gregorian::date&> : default_converter<boost::gregorian::date> { };

template <>
struct default_converter<boost::posix_time::ptime> : native_converter_base<boost::posix_time::ptime>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
  }

  boost::posix_time::ptime from(lua_State* L, int index)
  {
    double t = lua_tonumber(L, index);
    auto ptime = boost::posix_time::from_time_t(std::floor(t));
    ptime += boost::posix_time::microseconds(std::fmod(t, 1) * 1000000);
    return ptime;
  }
  
  void to(lua_State* L, const boost::posix_time::ptime& ptime)
  {
    struct tm tm = boost::posix_time::to_tm(ptime);
    time_t seconds = mktime(&tm);
    const auto& td = ptime.time_of_day();
    time_t microseconds = td.total_microseconds() - (td.total_seconds() * 1000000);
    lua_pushnumber(L, seconds + static_cast<double>(microseconds) / 1000000.0);
  }
};  

template <>
struct default_converter<const boost::posix_time::ptime&> : default_converter<boost::posix_time::ptime> { };

template <>
struct default_converter<const boost::posix_time::ptime> : default_converter<boost::posix_time::ptime> { };

template <>
struct default_converter<long long> : native_converter_base<long long>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
  }

  long long from(lua_State* L, int index)
  {
    return static_cast<long long>(lua_tonumber(L, index));
  }
  
  void to(lua_State* L, const long long& num)
  {
		lua_pushnumber(L, static_cast<lua_Number>(num));
  }
};  

template <>
struct default_converter<const long long&> : default_converter<long long> { };

template <>
struct default_converter<util::Error> : native_converter_base<util::Error>
{
  static int compute_score(lua_State* /* L */, int /* index */)
  {
    return 1;
  }

  util::Error from(lua_State* /* L */, int /* index */)
  {
    verify(false);
  }
  
  void to(lua_State* L, const util::Error& e)
  {
    if (!e)
    {
      lua_pushboolean(L, false);
      lua_pushstring(L, e.Message().c_str());
    }
    else
    {
      lua_pushboolean(L, true);
      lua_pushnil(L);
    }
  }
};  

template <>
struct default_converter<db::mail::Status> : native_converter_base<db::mail::Status>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
  }

  db::mail::Status from(lua_State* L, int index)
  {
    db::mail::Status status;
    if (!util::EnumFromString(lua_tostring(L, index), status))
      throw util::RuntimeError("Value erorr");
    return status;
  }
  
  void to(lua_State* L, db::mail::Status status)
  {
		lua_pushstring(L, util::EnumToString(status).c_str());
  }
};  

template <>
struct default_converter<ftp::xdupe::Mode> : native_converter_base<ftp::xdupe::Mode>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
  }

  ftp::xdupe::Mode from(lua_State* L, int index)
  {
    int i = static_cast<int>(lua_tonumber(L, index));
    if (i < static_cast<int>(ftp::xdupe::Mode::Disabled) ||
        i > static_cast<int>(ftp::xdupe::Mode::Four))
    {
      throw plugin::ValueError();
    }
    return static_cast<ftp::xdupe::Mode>(i);
  }
  
  void to(lua_State* L, ftp::xdupe::Mode mode)
  {
    lua_pushnumber(L, static_cast<int>(mode));
  }
};  

template <>
struct default_converter<ftp::ClientState> : native_converter_base<ftp::ClientState>
{
  static int compute_score(lua_State* /* L */, int /* index */)
  {
    return 1;
  }

  ftp::ClientState from(lua_State* /* L */, int /* index */)
  {
    verify(false);
  }
  
  void to(lua_State* L, ftp::ClientState state)
  {
    lua_pushnumber(L, static_cast<int>(state));
  }
};  

template <>
struct default_converter<plugin::HookID> : native_converter_base<plugin::HookID>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
  }

  plugin::HookID from(lua_State* L, int index)
  {
		plugin::HookID hookId;
		if (!plugin::HookIDFromString(lua_tostring(L, index), hookId))
			throw plugin::ValueError();
		return hookId;
  }
  
  void to(lua_State* L, plugin::HookID hookId)
  {
		lua_pushstring(L, plugin::HookIDToString(hookId).c_str());
  }
};  

template <>
struct default_converter<const plugin::HookID&> : default_converter<plugin::HookID> { };

template <>
struct default_converter<const plugin::HookID> : default_converter<plugin::HookID> { };
}

namespace lua
{

class HookArgVisitor : public boost::static_visitor<luabind::object>
{
  lua_State* L;
  
public:
  HookArgVisitor(lua_State* L) : L(L) { }

  template <typename T>
  luabind::object operator()(const T& value) { return luabind::object(L, value); }
};

class EventHookFunction
{
  luabind::object function;
  
public:
  EventHookFunction(const luabind::object& function) :
    function(function)
  {
    if (luabind::type(function) != LUA_TFUNCTION)
      throw plugin::ValueError();
  }

  plugin::HookResult operator()(plugin::Client& client, const plugin::EventHookArgs& args)
  {  
    lua_State* L = function.interpreter();

    // push function onto stack
    function.push(L);

    // push arguments onto stack
    luabind::object(L, client).push(L);
    HookArgVisitor visitor(L);
    for (auto& arg : args)
    {
      boost::apply_visitor(visitor, arg).push(L);
    }

    // call function
    int nargs = args.size() + 1;    
    if (lua_pcall(L, nargs, 1, 0) != 0)
    {
      logs::Error("Error while executing event hook: %1%", lua_tostring(L, -1));
      return plugin::HookResult::Failure;
    }
    
    std::cout << "???????" << std::endl;
    
    // process return value
    if (lua_isnil(L, -1))
    {
      lua_pop(L, 1);
      return plugin::HookResult::NoReturnValue;
    }
    
    plugin::HookResult result;
    bool okay = lua_isnumber(L, -1) && IntegerToHookResult(static_cast<int>(lua_tonumber(L, -1)), result);
    lua_pop(L, 1);
    
    if (!okay) return plugin::HookResult::InvalidReturnValue;
    return result;
  }  
};

class CommandHookFunction
{
  luabind::object function;
  
public:
  CommandHookFunction(const luabind::object& function) :
    function(function)
  {
    if (luabind::type(function) != LUA_TFUNCTION)
      throw plugin::ValueError();
  }
  
  plugin::HookResult operator()(plugin::Client& client, const std::string& commandLine, const std::vector<std::string>& args)
  {
    lua_State* L = function.interpreter();

    // push function onto stack
    function.push(L);

    // push arguments onto stack
    luabind::object(L, client).push(L);
    luabind::object(L, commandLine).push(L);
    luabind::object(L, args).push(L);

    // call function
    if (!lua_pcall(L, 3, 1, 0) != 0)
    {
      return plugin::HookResult::Failure;
    }
    
    // process return value
    if (lua_isnil(L, -1))
    {
      lua_pop(L, 1);
      return plugin::HookResult::NoReturnValue;
    }
    
    plugin::HookResult result;
    bool okay = lua_isnumber(L, -1) && IntegerToHookResult(static_cast<int>(lua_tonumber(L, -1)), result);
    lua_pop(L, 1);
    
    if (!okay) return plugin::HookResult::InvalidReturnValue;
    return result;
  }
};

plugin::HookID HookEvent(plugin::Event event, bool always, const luabind::object& function)
{
  return plugin::PluginState::Current().EventHooks().Connect(
            event, EventHookFunction(function), always);
}

void UnhookEvent(const plugin::HookID& hookId)
{
  plugin::PluginState::Current().EventHooks().Disconnect(hookId);
}

boost::optional<plugin::HookID> HookCommand(
        const std::string& command, const std::string& description, 
        const std::string& acl, const luabind::object& function)
{
  return plugin::PluginState::Current().CommandHooks().Connect(
            command, description, "", acl, CommandHookFunction(function));
}

void UnhookCommand(const plugin::HookID& hookId)
{
  plugin::PluginState::Current().CommandHooks().Disconnect(hookId);
}

void TranslateThreadInterrupted(lua_State* L, const boost::thread_interrupted&)
{
  lua_pushstring(L ,"Thread interrupted");
}


void ThreadInterruptionCheck(lua_State* L, lua_Debug* /* ar */)
{
  if (boost::this_thread::interruption_requested())
    luaL_error(L, "Thread interrupted");
}

const char* LuaStrerror(int error)
{
  switch (error)
  {
    case LUA_YIELD      : return "Yield error";
    case LUA_ERRRUN     : return "Runtime error";
    case LUA_ERRSYNTAX  : return "Syntax error";
    case LUA_ERRMEM     : return "Out of memory";
    case LUA_ERRERR     : return "Unable to run error handler";
    case LUA_ERRFILE    : return "Unable to open and/or read file";
  }
  
  return "Unknown error";
}

void DoBinding(lua_State* L)
{
  using luabind::class_;
  using luabind::module;
  using luabind::constructor;
  using luabind::def;
  using luabind::value;
  
  luabind::open(L);
  luabind::register_exception_handler<boost::thread_interrupted>(&TranslateThreadInterrupted);

  module(L, "ebftpd")
  [
    // User class
    
    class_<plugin::User>("User")
      .def(constructor<>())
      .property("uid",                    &plugin::User::ID)
      .property("name",                   &plugin::User::Name)
      .def("rename",                      &plugin::User::Rename)
      .property("ip_masks",               &plugin::User::IPMasks)
      .def("add_ip_mask",                 (bool(plugin::User::*)(const std::string&)) &plugin::User::AddIPMask)
      .def("del_ip_mask",                 (void(plugin::User::*)(const std::string&)) &plugin::User::DelIPMask)
      .def("del_ip_mask",                 (std::string(plugin::User::*)(size_t)) &plugin::User::DelIPMask)
      .def("clear_ip_masks",              &plugin::User::ClearIPMasks)
      .def("verify_password",             &plugin::User::VerifyPassword)
      .def("set_password",                &plugin::User::SetPassword)
      .property("flags",                  &plugin::User::Flags, &plugin::User::SetFlags)
      .def("add_flags",                   &plugin::User::AddFlags)
      .def("del_flags",                   &plugin::User::DelFlags)
      .property("primary_gid",            &plugin::User::PrimaryGID, &plugin::User::SetPrimaryGID)
      .property("primary_group",          &plugin::User::PrimaryGroup)
      .property("secondary_gids",         &plugin::User::SecondaryGIDs)
      .def("has_gid",                     &plugin::User::HasGID)
      .def("add_gids",                    &plugin::User::AddGIDs)
      .def("del_gids",                    &plugin::User::DelGIDs)
      .def("set_gids",                    &plugin::User::SetGIDs)
      .def("toggle_gids",                 &plugin::User::ToggleGIDs)
      .property("gadmin_gids",            &plugin::User::GadminGIDs)
      .def("has_gadmin_gid",              &plugin::User::HasGadminGID)
      .def("add_gadmin_gid",              &plugin::User::AddGadminGID)
      .def("del_gadmin_gid",              &plugin::User::DelGadminGID)
      .def("toggle_gadmin_gid",           &plugin::User::ToggleGadminGID)
      .property("creator",                &plugin::User::Creator)
      .property("created",                &plugin::User::Created)
      .property("home_dir",               &plugin::User::HomeDir)
      .def("set_home_dir",                &plugin::User::SetHomeDir)
      .property("idle_time",              &plugin::User::IdleTime)
      .def("set_idle_time",               &plugin::User::SetIdleTime)
      .property("expires",                &plugin::User::Expires)
      .def("set_expires",                 &plugin::User::SetExpires)
      .property("is_expired",             &plugin::User::Expired)
      .property("num_logins",             &plugin::User::NumLogins)
      .def("set_num_logins",              &plugin::User::SetNumLogins)
      .property("comment",                &plugin::User::Comment)
      .def("set_comment",                 &plugin::User::SetComment)
      .property("tagline",                &plugin::User::Tagline)
      .def("set_tagline",                 &plugin::User::SetTagline)
      .property("max_down_speed",         &plugin::User::MaxDownSpeed)
      .def("set_max_down_speed",          &plugin::User::SetMaxDownSpeed)
      .property("max_up_speed",           &plugin::User::MaxUpSpeed)
      .def("set_max_up_speed",            &plugin::User::SetMaxUpSpeed)
      .property("max_sim_down",           &plugin::User::MaxSimDown)
      .def("set_max_sim_down",            &plugin::User::SetMaxSimDown)
      .property("max_sim_up",             &plugin::User::MaxSimUp)
      .def("set_max_sim_up",              &plugin::User::SetMaxSimUp)
      .property("last_login",             &plugin::User::LastLogin)
      .property("default_ratio",          &plugin::User::DefaultRatio)
      .def("set_default_ratio",           &plugin::User::SetDefaultRatio)
      .def("get_section_ratio",           &plugin::User::SectionRatio)
      .def("set_section_ratio",           &plugin::User::SetSectionRatio)
      .property("default_credits",        &plugin::User::DefaultCredits)
      .def("incr_default_credits",        &plugin::User::IncrDefaultCredits)
      .def("decr_default_credits",        &plugin::User::DecrDefaultCredits)
      .def("decr_default_credits_force",  &plugin::User::DecrDefaultCreditsForce)
      .def("get_section_credits",         &plugin::User::SectionCredits)
      .def("incr_section_credits",        &plugin::User::IncrSectionCredits)
      .def("decr_section_credits",        &plugin::User::DecrSectionCredits)
      .def("decr_section_credits_force",  &plugin::User::DecrSectionCreditsForce)
      .property("default_allotment",      &plugin::User::DefaultWeeklyAllotment)
      .def("set_default_allotment",       &plugin::User::SetDefaultWeeklyAllotment)
      .def("set_section_allotment",       &plugin::User::SetSectionWeeklyAllotment)
      .def("get_section_allotment",       &plugin::User::SectionWeeklyAllotment)
      .def("purge",                       &plugin::User::Purge)
      .scope
      [
        def("load",         (boost::optional<plugin::User>(*)(acl::UserID)) &plugin::User::Load),
        def("load",         (boost::optional<plugin::User>(*)(const std::string& name)) &plugin::User::Load),
        def("create",       &plugin::User::Create),
        def("get_uids",     (std::vector<acl::UserID>(*)(const std::string&)) &plugin::User::GetUIDs),
        def("get_uids",     (std::vector<acl::UserID>(*)()) &plugin::User::GetUIDs),
        def("get_users",    (std::vector<plugin::User>(*)(const std::string&)) &plugin::User::GetUsers),
        def("get_users",    (std::vector<plugin::User>(*)()) &plugin::User::GetUsers),
        def("total_users",  &plugin::User::TotalUsers)
      ],
      
    // User utils
    def("name_to_uid",          &acl::NameToUID),
    def("uid_to_name",          &acl::UIDToName),
    def("uid_to_primary_gid",   &acl::UIDToPrimaryGID),
    def("name_to_primary_gid",  &acl::NameToPrimaryGID),
    def("uid_exists",           &acl::UIDExists),
    def("user_exists",          &acl::UserExists),
    def("name_to_gid",          &acl::NameToGID),
    
    // Group class
    
    class_<plugin::Group>("Group")
      .def(constructor<>())
      .property("gid",                &plugin::Group::ID)
      .property("name",               &plugin::Group::Name)
      .def("rename",                  &plugin::Group::Rename)
      .property("description",        &plugin::Group::Description)
      .def("set_description",         &plugin::Group::SetDescription)
      .property("comment",            &plugin::Group::Comment)
      .def("set_comment",             &plugin::Group::SetComment)
      .property("slots",              &plugin::Group::Slots)
      .def("set_slots",               &plugin::Group::SetSlots)
      .property("leech_slots",        &plugin::Group::LeechSlots)
      .def("set_leech_slots",         &plugin::Group::SetLeechSlots)
      .property("allotment_slots",    &plugin::Group::AllotmentSlots)
      .def("set_allotment_slots",     &plugin::Group::SetAllotmentSlots)
      .property("max_allotment_size", &plugin::Group::MaxAllotmentSize)
      .def("set_max_allotment_size",  &plugin::Group::SetMaxAllotmentSize)
      .property("max_logins",         &plugin::Group::MaxLogins)
      .def("set_max_logins",          &plugin::Group::SetMaxLogins)
      .def("slots_used",              &plugin::Group::NumSlotsUsed)
      .def("num_leeches",             &plugin::Group::NumLeeches)
      .def("num_members",             &plugin::Group::NumMembers)
      .def("num_allotments",          &plugin::Group::NumAllotments)
      .def("total_allotment_size",    &plugin::Group::TotalAllotmentSize)
      .def("purge",                   &plugin::Group::Purge)
      .scope
      [
        def("load",       (boost::optional<plugin::Group>(*)(acl::GroupID)) &plugin::Group::Load),
        def("load",       (boost::optional<plugin::Group>(*)(const std::string& name)) &plugin::Group::Load),
        def("get_gids",   (std::vector<acl::GroupID>(*)(const std::string&)) &plugin::Group::GetGIDs),
        def("get_gids",   (std::vector<acl::GroupID>(*)()) &plugin::Group::GetGIDs),
        def("get_groups", (std::vector<plugin::Group>(*)(const std::string&)) &plugin::Group::GetGroups),
        def("get_groups", (std::vector<plugin::Group>(*)()) &plugin::Group::GetGroups)
      ],
    
    // Group utils
    def("gid_to_name",  &acl::GIDToName),
    def("gid_exists",   &acl::GIDExists),
    def("group_exists", &acl::GroupExists),
    
    class_<fs::Owner>("Owner")
      .def(constructor<acl::UserID, acl::GroupID>())
      .property("uid", &fs::Owner::UID)
      .property("gid", &fs::Owner::GID),
    
    def("set_owner", (util::Error(*)(const std::string&, const fs::Owner&)) &fs::SetOwner),
    def("get_owner", (fs::Owner(*)(const std::string&)) &fs::GetOwner),

    // Message class
    class_<db::mail::Message>("Message")
      .def(constructor<const std::string&, acl::UserID, const std::string&,
                       const boost::posix_time::ptime&>())
/*      .scope
      [
        class_<plugin::MailStatus>("Status")
      ]*/
      .property("sender",     &db::mail::Message::Sender)
      .property("recipent",   &db::mail::Message::Recipient)
      .property("body",       &db::mail::Message::Body)
      .property("time_sent",  &db::mail::Message::TimeSent)
      .property("status",     &db::mail::Message::Status)
      .def("trash",           &db::mail::Trash),
    
    def("send_message",         &db::mail::Send),
    def("get_messages",         &db::mail::Get),
    def("save_message",         &db::mail::Save),
    def("save_message_trash",   &db::mail::SaveTrash),
    def("empty_message_trash",  &db::mail::PurgeTrash),
    
    def("hook_event",   &HookEvent),
    def("unhook_event", &UnhookEvent),
    
    class_<plugin::Event>("Event")
      .enum_("constants")
      [
        value("connected",          static_cast<int>(plugin::Event::Connected)),
        value("disconnected",       static_cast<int>(plugin::Event::Disconnected)),
        value("logged_in",          static_cast<int>(plugin::Event::LoggedIn)),
        value("logged_out",         static_cast<int>(plugin::Event::LoggedOut)),
        value("before_command",     static_cast<int>(plugin::Event::BeforeCommand)),
        value("after_command_okay", static_cast<int>(plugin::Event::AfterCommandOkay)),
        value("after_command_fail", static_cast<int>(plugin::Event::AfterCommandFail)),
        value("before_upload",      static_cast<int>(plugin::Event::BeforeUpload)),
        value("upload_okay",        static_cast<int>(plugin::Event::AfterUploadOkay)),
        value("upload_fail",        static_cast<int>(plugin::Event::AfterUploadFail)),
        value("before_download",    static_cast<int>(plugin::Event::BeforeDownload)),
        value("download_okay",      static_cast<int>(plugin::Event::AfterDownloadOkay)),
        value("download_fail",      static_cast<int>(plugin::Event::AfterDownloadFail))
      ],
      
    def("hook_command",   &HookCommand),
    def("unhook_command", &UnhookCommand),
    
    class_<plugin::Client>("Client")
      .def(constructor<>())
      .property("logged_in_at",           &plugin::Client::LoggedInAt)
      .property("idle_time",              &plugin::Client::IdleTime)
      .property("idle_timeout",           &plugin::Client::IdleTimeout)
      .def("set_idle_timeout",            &plugin::Client::SetIdleTimeout)
      .property("state",                  &plugin::Client::State)
      .property("user",                   (boost::optional<plugin::User&> (plugin::Client::*)()) &plugin::Client::User)
      .property("xdupe_mode",             &plugin::Client::XDupeMode)
      .def("set_xdupe_mode",              &plugin::Client::SetXDupeMode)
      .property("ip",                     &plugin::Client::IP)
      .property("ident",                  &plugin::Client::Ident)
      .property("hostname",               &plugin::Client::Hostname)
      .property("single_line_replies",    &plugin::Client::SingleLineReplies)
      .def("set_single_line_replies",     &plugin::Client::SetSingleLineReplies)
      .def("reply",                       &plugin::Client::Reply)
      .def("part_reply",                  &plugin::Client::PartReply)
      .scope
      [
        class_<ftp::ClientState>("State")
          .enum_("constants")
          [
            value("logged_out",         static_cast<int>(ftp::ClientState::LoggedOut)),
            value("waiting_password",   static_cast<int>(ftp::ClientState::WaitingPassword)),
            value("logged_in",          static_cast<int>(ftp::ClientState::LoggedIn)),
            value("finished",           static_cast<int>(ftp::ClientState::Finished))
          ],
          
        class_<ftp::xdupe::Mode>("XDupeMode")
          .enum_("constants")
          [
            value("disabled",           static_cast<int>(ftp::xdupe::Mode::Disabled)),
            value("one",                static_cast<int>(ftp::xdupe::Mode::One)),
            value("two",                static_cast<int>(ftp::xdupe::Mode::Two)),
            value("three",              static_cast<int>(ftp::xdupe::Mode::Three)),
            value("four",               static_cast<int>(ftp::xdupe::Mode::Four))
          ]
      ]
  ];
  
  // setup string enumerations for message status at Message.Status.enum
 /* {
    luabind::object global = luabind::globals(L);
    luabind::object status = global["ebftpd"]["Message"]["Status"];
    status["unread"] = "unread";
    status["trash"] = "trash";
    status["saved"] = "saved";
  }*/
}

Plugin* Factory::Create(const plugin::PluginDriver& driver) const
{
  return new Plugin(driver);
}

void Plugin::Initialise()
{
  verify(!L);
  L = luaL_newstate();
  if (!L) throw plugin::InitialiseError(LuaStrerror(LUA_ERRMEM));
  lua_sethook(L, &ThreadInterruptionCheck, LUA_MASKCOUNT, 1);
  luaL_openlibs(L);
  DoBinding(L);
  LoadScripts();
}

void Plugin::Cleanup()
{
  commandHooks = nullptr;
  eventHooks = nullptr;
  
  if (L)
  {
    lua_close(L);
    L = nullptr;
  }
}

void Plugin::LoadScript(const std::string& path)
{
  int error = luaL_loadfile(L, path.c_str());
  if (error != 0) throw plugin::PluginError(LuaStrerror(error));
  error = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (error != 0)
  {
    if (error == LUA_ERRRUN)
    {
      std::ostringstream os;
      os << LuaStrerror(LUA_ERRRUN) << ": " << lua_tostring(L, -1) << std::endl;
      throw plugin::PluginError(os.str());
    }
    else
      throw plugin::PluginError(LuaStrerror(error));
  }
}

} /* lua namespace */

extern "C" lua::Factory* CreateFactory()
{
  return new lua::Factory();
}

#ifdef __LUA_TEST

int main(int argc, char** argv)
{
  assert(argc == 2);
  
  try
  {
    cfg::UpdateShared(cfg::Config::Load("/home/bioboy/dev/ebftpd/ebftpd.conf", true));
  }
  catch (const cfg::ConfigError& e)
  {
    std::cerr << "config load error: " << e.what() << std::endl;
    return 1;
  }  

  try
  {
    std::unique_ptr<plugin::Factory> factory(new lua::Factory());
    std::unique_ptr<plugin::Plugin> plugin(factory->Create());
    plugin->LoadScript(argv[1]);
  }
  catch (const plugin::PluginError& e)
  {
    std::cout << e.what() << std::endl;
  }
}

#endif
