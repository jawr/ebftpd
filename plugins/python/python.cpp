#include <Python.h>
#include <datetime.h>
#include <boost/python.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "python/python.hpp"
#include "acl/user.hpp"
#include "util/verify.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"

namespace scripting { namespace python
{

namespace py = boost::python;

template <typename T, typename TfromPy>
struct object_from_python
{
   object_from_python()
   {
     py::converter::registry::push_back(&TfromPy::convertible, &TfromPy::construct, py::type_id<T>());
   }
};

template <typename T, typename TtoPy, typename TfromPy>
struct register_python_conversion
{
   register_python_conversion()
   {
     py::to_python_converter<T, TtoPy>();
     object_from_python<T, TfromPy>();
   }
};

template <typename T>
struct python_optional : public boost::noncopyable
{
  struct optional_to_python
  {
    static PyObject * convert(const boost::optional<T>& value)
    {
      return (value ? py::to_python_value<T>()(*value) : py::detail::none());
    }
  };

  struct optional_from_python
  {
    static void * convertible(PyObject * source)
    {
      using namespace py::converter;
      
      if (source == Py_None) return source;

      const registration& converters(registered<T>::converters);

      if (implicit_rvalue_convertible_from_python(source, converters))
      {
        rvalue_from_python_stage1_data data = rvalue_from_python_stage1(source, converters);
        return rvalue_from_python_stage2(source, data, converters);
      }
      
      return nullptr;
    }

    static void construct(PyObject * source, py::converter::rvalue_from_python_stage1_data * data)
    {
      using namespace py::converter;

      void * const storage = ((rvalue_from_python_storage<T> *)data)->storage.bytes;

      if (data->convertible == source)	    // == None
        new (storage) boost::optional<T>(); // A Boost uninitialized value
      else
        new (storage) boost::optional<T>(*static_cast<T *>(data->convertible));
        data->convertible = storage;
    }
  };

  explicit python_optional()
  {
    register_python_conversion<boost::optional<T>, optional_to_python, optional_from_python>();
  }
};

template <typename C>
struct iterable_to_python_list
{
    static PyObject* convert(const C& cont)
    {
        py::list l;
        for (auto& elem : cont) l.append(elem);
        return py::incref(l.ptr());
    }
    
    iterable_to_python_list()
    {
      py::to_python_converter<C, iterable_to_python_list<C>>();
    }
};

template <typename C>
struct back_insertable_from_python_seq
{
  back_insertable_from_python_seq()
  {
    py::converter::registry::push_back(&convertible, &construct, py::type_id<C>());
  }

  static void* convertible(PyObject * obj_ptr)
  {
    if (!PySequence_Check(obj_ptr)) return nullptr;
    return obj_ptr;
  }

  static void construct(PyObject* obj_ptr, py::converter::rvalue_from_python_stage1_data* data)
  {
    void* storage = ((py::converter::rvalue_from_python_storage<C>*)data)->storage.bytes;
    new (storage) C();
    C* container = reinterpret_cast<C*>(storage);
    int size = PySequence_Size(obj_ptr);
    for (int i = 0; i < size; ++i)
    {
      container->push_back(py::extract<typename C::value_type>(PySequence_GetItem(obj_ptr, i)));
    }
    data->convertible = storage;
  }
};

struct ptime_to_python_datetime
{
  static PyObject* convert(const boost::posix_time::ptime& pt)
  {
    boost::gregorian::date date = pt.date();
    boost::posix_time::time_duration td = pt.time_of_day();
    return PyDateTime_FromDateAndTime(
            static_cast<int>(date.year()),
					  static_cast<int>(date.month()),
					  static_cast<int>(date.day()),
					  td.hours(),
					  td.minutes(),
					  td.seconds(),
					  td.total_microseconds() - td.total_seconds() * 1000000);
  }
  
  ptime_to_python_datetime()
  {
    py::to_python_converter<boost::posix_time::ptime, ptime_to_python_datetime>();
  }
};

struct ptime_from_python_datetime
{
   ptime_from_python_datetime()
   {
     py::converter::registry::push_back(&convertible, &construct, py::type_id<boost::posix_time::ptime>());
   }

   static void* convertible(PyObject * obj_ptr)
   {
     if (!PyDateTime_Check(obj_ptr)) return nullptr;
     return obj_ptr;
   }

   static void construct(PyObject* obj_ptr, py::converter::rvalue_from_python_stage1_data* data)
   {
     PyDateTime_DateTime const* pydate = reinterpret_cast<PyDateTime_DateTime*>(obj_ptr);

     // Create date object
     boost::gregorian::date _date(PyDateTime_GET_YEAR(pydate),
                                  PyDateTime_GET_MONTH(pydate),
                                  PyDateTime_GET_DAY(pydate));

     // Create time duration object
     boost::posix_time::time_duration _duration(PyDateTime_DATE_GET_HOUR(pydate),
                                                PyDateTime_DATE_GET_MINUTE(pydate),
                                                PyDateTime_DATE_GET_SECOND(pydate), 0);
     // Set the usecs value
     _duration += boost::posix_time::microseconds(PyDateTime_DATE_GET_MICROSECOND(pydate));

     // Create posix time object
     void* storage = ((py::converter::rvalue_from_python_storage<boost::posix_time::ptime>*)data)->storage.bytes;
     new (storage) boost::posix_time::ptime(_date, _duration);
     data->convertible = storage;
   }
};

struct gdate_to_python_date
{
  static PyObject* convert(const boost::gregorian::date& date)
  {
    return PyDate_FromDate(
            static_cast<int>(date.year()),
					  static_cast<int>(date.month()),
					  static_cast<int>(date.day()));
  }

  gdate_to_python_date()
  {
    py::to_python_converter<boost::gregorian::date, gdate_to_python_date>();
  }
};

struct gdate_from_python_date
{
   gdate_from_python_date()
   {
     py::converter::registry::push_back(&convertible, &construct, py::type_id<boost::gregorian::date>());
   }

   static void* convertible(PyObject * obj_ptr)
   {
     if (!PyDate_Check(obj_ptr)) return nullptr;
     return obj_ptr;
   }

   static void construct(PyObject* obj_ptr, py::converter::rvalue_from_python_stage1_data* data)
   {
     PyDateTime_Date const* pydate = reinterpret_cast<PyDateTime_Date*>(obj_ptr);

     // Create date object
     // Create posix time object
     void* storage = ((py::converter::rvalue_from_python_storage<boost::gregorian::date>*)data)->storage.bytes;
     new (storage) boost::gregorian::date(PyDateTime_GET_YEAR(pydate),
																					PyDateTime_GET_MONTH(pydate),
																					PyDateTime_GET_DAY(pydate));
     data->convertible = storage;
   }
};

void Test()
{
}

BOOST_PYTHON_MODULE(ebftpd)
{
  
  PyDateTime_IMPORT;
  python_optional<acl::User>();
  python_optional<boost::posix_time::ptime>();
  ptime_to_python_datetime();
  gdate_to_python_date();
  iterable_to_python_list<std::vector<std::string>>();
  iterable_to_python_list<std::unordered_set<acl::GroupID>>();
  iterable_to_python_list<std::vector<acl::GroupID>>();
  iterable_to_python_list<std::vector<acl::User>>();
  back_insertable_from_python_seq<std::vector<acl::GroupID>>();
  
  std::function<void()> test = &Test;

 boost::optional<acl::User> (*LoadUserUID)(acl::UserID uid) = &acl::User::Load;
  boost::optional<acl::User> (*LoadUserName)(const std::string& name) = &acl::User::Load;
  std::string (acl::User::*DelIPMaskIndex)(size_t) = &acl::User::DelIPMask;
  void (acl::User::*DelIPMaskValue)(const std::string&) = &acl::User::DelIPMask;  
  bool (acl::User::*AddIPMask)(const std::string&) = &acl::User::AddIPMask;  
  std::vector<acl::UserID> (*GetUIDs)(const std::string&) = &acl::User::GetUIDs;
  std::vector<acl::UserID> (*GetUIDsAll)() = &acl::User::GetUIDs;
  std::vector<acl::User> (*GetUsers)(const std::string&) = &acl::User::GetUsers;
  std::vector<acl::User> (*GetUsersAll)() = &acl::User::GetUsers;

  py::class_<acl::User>("User", py::no_init)
    .add_property("name", py::make_function(&acl::User::Name, py::return_value_policy<py::copy_const_reference>()))
    .def("rename", &acl::User::Rename)
    .add_property("ip_masks", py::make_function(&acl::User::IPMasks, py::return_value_policy<py::copy_const_reference>()))
    .def("add_ip_mask", AddIPMask)
    .def("del_ip_mask", DelIPMaskIndex)
    .def("del_ip_mask", DelIPMaskValue)
    .def("clear_ip_masks", &acl::User::ClearIPMasks)
    .def("verify_password", &acl::User::VerifyPassword)
    .def("set_password", &acl::User::SetPassword)
    .add_property("flags", py::make_function(&acl::User::Flags, py::return_value_policy<py::copy_const_reference>()),
                           py::make_function(&acl::User::SetFlags))
    .def("add_flags", &acl::User::AddFlags)
    .def("del_flags", &acl::User::DelFlags)
    .add_property("primary_gid", &acl::User::PrimaryGID, &acl::User::SetPrimaryGID)
    .add_property("primary_group", &acl::User::PrimaryGroup)
    .def("secondary_gids", &acl::User::SecondaryGIDs)
    .def("has_gid", &acl::User::HasGID)
    .def("add_gids", &acl::User::AddGIDs)
    .def("del_gids", &acl::User::DelGIDs)
    .def("set_gids", &acl::User::SetGIDs)
    .def("toggle_gids", &acl::User::ToggleGIDs)
    .add_property("gadmin_gids", &acl::User::GadminGIDs)
    .def("has_gadmin_gid", &acl::User::HasGadminGID)
    .def("add_gadmin_gid", &acl::User::AddGadminGID)
    .def("del_gadmin_gid", &acl::User::DelGadminGID)
    .def("toggle_gadmin_gid", &acl::User::ToggleGadminGID)
    .add_property("creator", &acl::User::Creator)
    .add_property("created", py::make_function(&acl::User::Created, py::return_value_policy<py::copy_const_reference>()))
    .add_property("home_dir", py::make_function(&acl::User::HomeDir, py::return_value_policy<py::copy_const_reference>()),
                              py::make_function(&acl::User::SetHomeDir))
    .add_property("idle_time", &acl::User::IdleTime, &acl::User::SetIdleTime)
    .add_property("expires", py::make_function(&acl::User::Expires, py::return_value_policy<py::copy_const_reference>()),
                             py::make_function(&acl::User::SetExpires))
    .add_property("expired", &acl::User::Expired)
    .add_property("num_logins", &acl::User::NumLogins, &acl::User::SetNumLogins)
    .add_property("comment", py::make_function(&acl::User::Comment, py::return_value_policy<py::copy_const_reference>()),
                             py::make_function(&acl::User::SetComment))
    .add_property("tagline", py::make_function(&acl::User::Tagline, py::return_value_policy<py::copy_const_reference>()),
                             py::make_function(&acl::User::SetTagline))
    .add_property("max_down_speed", &acl::User::MaxDownSpeed, &acl::User::SetMaxDownSpeed)
    .add_property("max_up_speed", &acl::User::MaxUpSpeed, &acl::User::SetMaxUpSpeed)
    .add_property("max_sim_down", &acl::User::MaxSimDown, &acl::User::SetMaxSimDown)
    .add_property("max_sim_up", &acl::User::MaxSimUp, &acl::User::SetMaxSimUp)
    .add_property("last_login", py::make_function(&acl::User::LastLogin, py::return_value_policy<py::copy_const_reference>()))
    .add_property("default_ratio", &acl::User::DefaultRatio, &acl::User::SetDefaultRatio)
    .def("get_section_ratio", &acl::User::SectionRatio)
    .def("set_section_ratio", &acl::User::SetSectionRatio)
    .add_property("default_credits", &acl::User::DefaultCredits)
    .def("incr_default_credits", &acl::User::IncrDefaultCredits)
    .def("decr_default_credits", &acl::User::DecrDefaultCredits)
    .def("decr_default_credits_force", &acl::User::DecrDefaultCreditsForce)
    .def("get_section_credits", &acl::User::SectionCredits)
    .def("incr_section_credits", &acl::User::IncrSectionCredits)
    .def("decr_section_credits", &acl::User::DecrSectionCredits)
    .def("decr_section_credits_force", &acl::User::DecrSectionCreditsForce)
    .add_property("default_allotment", &acl::User::DefaultWeeklyAllotment,
                                       &acl::User::SetDefaultWeeklyAllotment)
    .def("set_section_allotment", &acl::User::SetSectionWeeklyAllotment)
    .def("get_section_allotment", &acl::User::SectionWeeklyAllotment)
    .def("purge", &acl::User::Purge)
    .def("load", LoadUserUID)
    .def("load", LoadUserName).staticmethod("load")
    .def("get_uids", GetUIDs)
    .def("get_uids", GetUIDsAll).staticmethod("get_uids")
    .def("get_users", GetUsers)
    .def("get_users", GetUsersAll).staticmethod("get_users")
    .def("total_users", &acl::User::TotalUsers).staticmethod("total_users")
    .def("test", py::make_function(test)).staticmethod("test")
  ;
}

} /* python namespace */
} /* scripting namespace */


int main(int argc, char** argv)
{
  assert(argc == 2);
  using namespace scripting::python;
  
  try
  {
    cfg::UpdateShared(cfg::Config::Load("/home/bioboy/dev/ebftpd/ebftpd.conf", true));
  }
  catch (const cfg::ConfigError& e)
  {
    std::cerr << "config load error: " << e.what() << std::endl;
    return 1;
  }

  Py_Initialize();
  PyEval_InitThreads();
  
  verify(PyImport_AppendInittab("ebftpd", initebftpd) != -1);

  try
  {
    auto main = py::import("__main__");
    auto global = main.attr("__dict__");
    auto result = py::exec_file(argv[1], global, global);
  }
  catch (const py::error_already_set& e)
  {
    //auto errmsg = parse_python_exception();
    //std::cerr << errmsg << std::endl;
    PyErr_Print();
  }  
  
  //Py_Finalize();
}
