#include <cmath>
#include <iostream>
#include <cassert>
#include <lua.hpp>
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
#include "script/user.hpp"
#include "script/group.hpp"
#include "script/mail.hpp"
#include "script/hooks.hpp"
#include "script/error.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "fs/owner.hpp"
#include "db/mail/message.hpp"
#include "db/mail/mail.hpp"

namespace luabind
{

template <typename T>
struct default_converter<boost::optional<T>> : native_converter_base<boost::optional<T>>
{
  static int compute_score(lua_State* L, int index)
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
  static int compute_score(lua_State* L, int index)
  {
    return 1;
  }

  util::Error from(lua_State* L, int index)
  {
    return util::Error::Failure("Should never get here");
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
struct default_converter<script::HookID> : native_converter_base<script::HookID>
{
  static int compute_score(lua_State* L, int index)
  {
    return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
  }

  script::HookID from(lua_State* L, int index)
  {
		script::HookID hookId;
		if (!script::HookIDFromString(lua_tostring(L, index), hookId))
			throw script::ValueError();
		return hookId;
  }
  
  void to(lua_State* L, script::HookID hookId)
  {
		lua_pushstring(L, script::HookIDToString(hookId).c_str());
  }
};  

template <>
struct default_converter<const script::HookID&> : default_converter<script::HookID> { };

template <>
struct default_converter<const script::HookID> : default_converter<script::HookID> { };
}

namespace script { namespace lua
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
      throw ValueError();
  }

  HookResult operator()(const Client& client, const EventHookArgs& args)
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
    if (!lua_pcall(L, nargs, 1, 0) != 0)
    {
      return HookResult::Failure;
    }
    
    // process return value
    if (lua_isnil(L, -1))
    {
      lua_pop(L, 1);
      return HookResult::NoReturnValue;
    }
    
    HookResult result;
    bool okay = lua_isnumber(L, -1) && IntegerToHookResult(static_cast<int>(lua_tonumber(L, -1)), result);
    lua_pop(L, 1);
    
    if (!okay) return HookResult::InvalidReturnValue;
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
      throw ValueError();
  }
  
  HookResult operator()(Client& client, const std::string& commandLine, const std::vector<std::string>& args)
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
      return HookResult::Failure;
    }
    
    // process return value
    if (lua_isnil(L, -1))
    {
      lua_pop(L, 1);
      return HookResult::NoReturnValue;
    }
    
    HookResult result;
    bool okay = lua_isnumber(L, -1) && IntegerToHookResult(static_cast<int>(lua_tonumber(L, -1)), result);
    lua_pop(L, 1);
    
    if (!okay) return HookResult::InvalidReturnValue;
    return result;
  }
};

HookID HookEvent(Event event, const luabind::object& function)
{
  return Hooks::Get().ConnectEvent(event, EventHookFunction(function));
}

void UnhookEvent(const HookID& hookId)
{
  Hooks::Get().DisconnectEvent(hookId);
}

boost::optional<HookID> HookCommand(const std::string& command, const std::string& description, 
                                    const std::string& acl, const luabind::object& function)
{
  return Hooks::Get().ConnectCommand(command, description, acl, CommandHookFunction(function));
}

void UnhookCommand(const HookID& hookId)
{
  Hooks::Get().DisconnectCommand(hookId);
}

void TranslateThreadInterrupted(lua_State* L, const boost::thread_interrupted&)
{
  std::cout << "WTH?" << std::endl;
  lua_pushstring(L ,"Thread interrupted");
}

extern "C" int init(lua_State* L)
{
  using luabind::class_;
  using luabind::module;
  using luabind::constructor;
  using luabind::def;
  using luabind::value;
  
  typedef script::User<script::NoUnlocking> User;
  typedef script::Group<script::NoUnlocking> Group;
  
  luabind::open(L);
  luabind::register_exception_handler<boost::thread_interrupted>(&TranslateThreadInterrupted);

  module(L, "ebftpd")
  [
    // User class
    
    class_<User>("User")
      .def(constructor<>())
      .property("uid",                    &User::ID)
      .property("name",                   &User::Name)
      .def("rename",                      &User::Rename)
      .property("ip_masks",               &User::IPMasks)
      .def("add_ip_mask",                 (bool(User::*)(const std::string&)) &User::AddIPMask)
      .def("del_ip_mask",                 (void(User::*)(const std::string&)) &User::DelIPMask)
      .def("del_ip_mask",                 (std::string(User::*)(size_t)) &User::DelIPMask)
      .def("clear_ip_masks",              &User::ClearIPMasks)
      .def("verify_password",             &User::VerifyPassword)
      .def("set_password",                &User::SetPassword)
      .property("flags",                  &User::Flags, &User::SetFlags)
      .def("add_flags",                   &User::AddFlags)
      .def("del_flags",                   &User::DelFlags)
      .property("primary_gid",            &User::PrimaryGID, &User::SetPrimaryGID)
      .property("primary_group",          &User::PrimaryGroup)
      .property("secondary_gids",         &User::SecondaryGIDs)
      .def("has_gid",                     &User::HasGID)
      .def("add_gids",                    &User::AddGIDs)
      .def("del_gids",                    &User::DelGIDs)
      .def("set_gids",                    &User::SetGIDs)
      .def("toggle_gids",                 &User::ToggleGIDs)
      .property("gadmin_gids",            &User::GadminGIDs)
      .def("has_gadmin_gid",              &User::HasGadminGID)
      .def("add_gadmin_gid",              &User::AddGadminGID)
      .def("del_gadmin_gid",              &User::DelGadminGID)
      .def("toggle_gadmin_gid",           &User::ToggleGadminGID)
      .property("creator",                &User::Creator)
      .property("created",                &User::Created)
      .property("home_dir",               &User::HomeDir)
      .def("set_home_dir",                &User::SetHomeDir)
      .property("idle_time",              &User::IdleTime)
      .def("set_idle_time",               &User::SetIdleTime)
      .property("expires",                &User::Expires)
      .def("set_expires",                 &User::SetExpires)
      .property("is_expired",             &User::Expired)
      .property("num_logins",             &User::NumLogins)
      .def("set_num_logins",              &User::SetNumLogins)
      .property("comment",                &User::Comment)
      .def("set_comment",                 &User::SetComment)
      .property("tagline",                &User::Tagline)
      .def("set_tagline",                 &User::SetTagline)
      .property("max_down_speed",         &User::MaxDownSpeed)
      .def("set_max_down_speed",          &User::SetMaxDownSpeed)
      .property("max_up_speed",           &User::MaxUpSpeed)
      .def("set_max_up_speed",            &User::SetMaxUpSpeed)
      .property("max_sim_down",           &User::MaxSimDown)
      .def("set_max_sim_down",            &User::SetMaxSimDown)
      .property("max_sim_up",             &User::MaxSimUp)
      .def("set_max_sim_up",              &User::SetMaxSimUp)
      .property("last_login",             &User::LastLogin)
      .property("default_ratio",          &User::DefaultRatio)
      .def("set_default_ratio",           &User::SetDefaultRatio)
      .def("get_section_ratio",           &User::SectionRatio)
      .def("set_section_ratio",           &User::SetSectionRatio)
      .property("default_credits",        &User::DefaultCredits)
      .def("incr_default_credits",        &User::IncrDefaultCredits)
      .def("decr_default_credits",        &User::DecrDefaultCredits)
      .def("decr_default_credits_force",  &User::DecrDefaultCreditsForce)
      .def("get_section_credits",         &User::SectionCredits)
      .def("incr_section_credits",        &User::IncrSectionCredits)
      .def("decr_section_credits",        &User::DecrSectionCredits)
      .def("decr_section_credits_force",  &User::DecrSectionCreditsForce)
      .property("default_allotment",      &User::DefaultWeeklyAllotment)
      .def("set_default_allotment",       &User::SetDefaultWeeklyAllotment)
      .def("set_section_allotment",       &User::SetSectionWeeklyAllotment)
      .def("get_section_allotment",       &User::SectionWeeklyAllotment)
      .def("purge",                       &User::Purge)
      .scope
      [
        def("load",         (boost::optional<User>(*)(acl::UserID)) &User::Load),
        def("load",         (boost::optional<User>(*)(const std::string& name)) &User::Load),
        def("create",       &User::Create),
        def("get_uids",     (std::vector<acl::UserID>(*)(const std::string&)) &User::GetUIDs),
        def("get_uids",     (std::vector<acl::UserID>(*)()) &User::GetUIDs),
        def("get_users",    (std::vector<User>(*)(const std::string&)) &User::GetUsers),
        def("get_users",    (std::vector<User>(*)()) &User::GetUsers),
        def("total_users",  &User::TotalUsers)
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
    
    class_<Group>("Group")
      .def(constructor<>())
      .property("gid",                &Group::ID)
      .property("name",               &Group::Name)
      .def("rename",                  &Group::Rename)
      .property("description",        &Group::Description)
      .def("set_description",         &Group::SetDescription)
      .property("comment",            &Group::Comment)
      .def("set_comment",             &Group::SetComment)
      .property("slots",              &Group::Slots)
      .def("set_slots",               &Group::SetSlots)
      .property("leech_slots",        &Group::LeechSlots)
      .def("set_leech_slots",         &Group::SetLeechSlots)
      .property("allotment_slots",    &Group::AllotmentSlots)
      .def("set_allotment_slots",     &Group::SetAllotmentSlots)
      .property("max_allotment_size", &Group::MaxAllotmentSize)
      .def("set_max_allotment_size",  &Group::SetMaxAllotmentSize)
      .property("max_logins",         &Group::MaxLogins)
      .def("set_max_logins",          &Group::SetMaxLogins)
      .def("slots_used",              &Group::NumSlotsUsed)
      .def("num_leeches",             &Group::NumLeeches)
      .def("num_members",             &Group::NumMembers)
      .def("num_allotments",          &Group::NumAllotments)
      .def("total_allotment_size",    &Group::TotalAllotmentSize)
      .def("purge",                   &Group::Purge)
      .scope
      [
        def("load",       (boost::optional<Group>(*)(acl::GroupID)) &Group::Load),
        def("load",       (boost::optional<Group>(*)(const std::string& name)) &Group::Load),
        def("get_gids",   (std::vector<acl::GroupID>(*)(const std::string&)) &Group::GetGIDs),
        def("get_gids",   (std::vector<acl::GroupID>(*)()) &Group::GetGIDs),
        def("get_groups", (std::vector<Group>(*)(const std::string&)) &Group::GetGroups),
        def("get_groups", (std::vector<Group>(*)()) &Group::GetGroups)
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
      .scope
      [
        class_<MailStatus>("Status")
      ]
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
    
    class_<script::Event>("Event")
      .enum_("constants")
      [
        value("logged_in",          static_cast<int>(Event::LoggedIn)),
        value("logged_out",         static_cast<int>(Event::LoggedOut)),
        value("before_command",     static_cast<int>(Event::BeforeCommand)),
        value("after_command_okay", static_cast<int>(Event::AfterCommandOkay)),
        value("after_command_fail", static_cast<int>(Event::AfterCommandFail)),
        value("before_upload",      static_cast<int>(Event::BeforeUpload)),
        value("after_upload",       static_cast<int>(Event::AfterUpload)),
        value("upload_okay",        static_cast<int>(Event::UploadOkay)),
        value("upload_fail",        static_cast<int>(Event::UploadFail)),
        value("before_download",    static_cast<int>(Event::BeforeDownload)),
        value("after_download",     static_cast<int>(Event::AfterDownload)),
        value("download_okay",      static_cast<int>(Event::DownloadOkay)),
        value("download_fail",      static_cast<int>(Event::DownloadFail))
      ],
      
    def("hook_command",   &HookCommand),
    def("unhook_command", &UnhookCommand)
  ];
  
  // setup string enumerations for message status at Message.Status.enum
  {
    luabind::object global = luabind::globals(L);
    luabind::object status = global["ebftpd"]["Message"]["Status"];
    status["unread"] = "unread";
    status["trash"] = "trash";
    status["saved"] = "saved";
  }
  
  return 0;
}

} /* lua namespace */
} /* script namespace */

void CountHook(lua_State* L, lua_Debug* ar)
{
  if (boost::this_thread::interruption_requested())
    luaL_error(L, "Thread interrupted");
}

void ThreadMain(const std::string& file, lua_State* L)
{
  luaL_loadfile(L, file.c_str());
  if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
  {
    std::cout << lua_tostring(L, -1) << std::endl;
  }
  //luabind::object script(luabind::from_stack(L, -1));
/*  try
  {
    luabind::call_function<void>(script);
  }
  catch (const luabind::error& e)
  {
    std::cout << e.what() << std::endl;
    std::cout << lua_tostring(L, -1) << std::endl;
  }*/
}

int main(int argc, char** argv)
{
  using namespace script;
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
  
  lua_State* L = luaL_newstate();
  lua_sethook(L, &CountHook, LUA_MASKCOUNT, 1);
  luaL_openlibs(L);
  {
    script::lua::init(L);

     boost::thread t(&ThreadMain, argv[1], L);
    /*luaL_loadfile(L, argv[1]);
    luabind::object script(luabind::from_stack(L, -1));
    try
    {
      luabind::call_function<void>(script);
    }
    catch (const luabind::error& e)
    {
      std::cout << e.what() << std::endl;
      std::cout << lua_tostring(L, -1) << std::endl;
    }
    /*
    if (luaL_dofile(L, argv[1]) != 0)
    {
      const char* errmsg = lua_tostring(L, -1);
      if (!strcmp(errmsg, "Thread interrupted"))
        throw boost::thread_interrupted();
      std::cout << lua_tostring(L, -1) << std::endl;
    }*/

  /*  EventHookArgs args = {1, 2, 3};
    script::Client client = 1;
    
    Hooks::Get().TriggerEvent(Event::LoggedIn, client, args);
    
    std::vector<std::string> cargs = { "SITE", "PRE", "one", "two", "three" };
    Hooks::Get().TriggerCommand(client, "SITE PRE one two three", cargs);
    */
    t.join();
    
    Hooks::Get().Clear();
  }
  
  lua_close(L);
  return 0;
}
