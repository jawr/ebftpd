#include <boost/algorithm/string.hpp>
#include "cmd/rfc/factory.hpp"
#include "cmd/rfc/abor.hpp"
#include "cmd/rfc/auth.hpp"
#include "cmd/rfc/cdup.hpp"
#include "cmd/rfc/cwd.hpp"
#include "cmd/rfc/dele.hpp"
#include "cmd/rfc/eprt.hpp"
#include "cmd/rfc/epsv.hpp"
#include "cmd/rfc/feat.hpp"
#include "cmd/rfc/help.hpp"
#include "cmd/rfc/list.hpp"
#include "cmd/rfc/lprt.hpp"
#include "cmd/rfc/lpsv.hpp"
#include "cmd/rfc/mdtm.hpp"
#include "cmd/rfc/mkd.hpp"
#include "cmd/rfc/mode.hpp"
#include "cmd/rfc/nlst.hpp"
#include "cmd/rfc/noop.hpp"
#include "cmd/rfc/pass.hpp"
#include "cmd/rfc/pasv.hpp"
#include "cmd/rfc/pbsz.hpp"
#include "cmd/rfc/port.hpp"
#include "cmd/rfc/prot.hpp"
#include "cmd/rfc/pwd.hpp"
#include "cmd/rfc/quit.hpp"
#include "cmd/rfc/retr.hpp"
#include "cmd/rfc/rmd.hpp"
#include "cmd/rfc/rnfr.hpp"
#include "cmd/rfc/rnto.hpp"
#include "cmd/rfc/site.hpp"
#include "cmd/rfc/size.hpp"
#include "cmd/rfc/stat.hpp"
#include "cmd/rfc/stor.hpp"
#include "cmd/rfc/stou.hpp"
#include "cmd/rfc/syst.hpp"
#include "cmd/rfc/type.hpp"
#include "cmd/rfc/user.hpp"
#include "cmd/rfc/sscn.hpp"

namespace cmd { namespace rfc
{

Factory Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "ABOR",   { 0,  0,  ftp::ClientState::AnyState,
                  CreatorBasePtr(new Creator<rfc::ABORCommand>()), "SITE ABOR" }, },
    { "ACCT",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "ADAT",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "ALLO",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "APPE",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "AUTH",   { 1,  1,  ftp::ClientState::LoggedOut,
                  CreatorBasePtr(new Creator<rfc::AUTHCommand>()), "SITE AUTH TLS|SSL" }, },
    { "CCC",    { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "CDUP",   { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::CDUPCommand>()), "CDUP" }, },
    { "CWD",    { 1,  -1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::CWDCommand>()), "CWD <path>" }, },
    { "DELE",   { 1,  -1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::DELECommand>()), "DELE <path>" }, },
    { "ENC",    { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "EPRT",   { 1,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::EPRTCommand>()), "EPRT <port-string>" }, },
    { "EPSV",   { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::EPSVCommand>()), "EPSV" }, },
    { "FEAT",   { 0,  0,  ftp::ClientState::AnyState,
                  CreatorBasePtr(new Creator<rfc::FEATCommand>()), "FEAT" }, },
    { "HELP",   { 0,  1,  ftp::ClientState::AnyState,
                  CreatorBasePtr(new Creator<rfc::HELPCommand>()), "HELP [<command>]" }, },
    { "LANG",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "LIST",   { 0,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::LISTCommand>()), "LIST [-<options>] [<path>]" }, },
    { "LPRT",   { 1,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::LPRTCommand>()), "LPRT <port-string>" }, },
    { "LPSV",   { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::LPSVCommand>()), "LPSV" }, },
    { "MDTM",   { 1,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::MDTMCommand>()), "MDTM <path>" }, },
    { "MIC",    { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "MKD",    { 1,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::MKDCommand>()), "MKD <path>" }, },
    { "MLSD",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "MLST",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "MODE",   { 1,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::MODECommand>()), "MODE S|B|C" }, },
    { "NLST",   { 0,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::NLSTCommand>()), "NLST [-<options>] [<path>]" }, },
    { "NOOP",   { 0,  0,  ftp::ClientState::AnyState,
                  CreatorBasePtr(new Creator<rfc::NOOPCommand>()), "NOOP" }, },
    { "OPTS",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "PASS",   { 0,  -1, ftp::ClientState::WaitingPassword,
                  CreatorBasePtr(new Creator<rfc::PASSCommand>()), "PASS <password>" }, },
    { "PASV",   { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::PASVCommand>()), "PASV" }, },
    { "PORT",   { 1,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::PORTCommand>()), "PORT <port-string>" }, },
    { "PBSZ",   { 1,  1,  ftp::ClientState::LoggedOut,
                  CreatorBasePtr(new Creator<rfc::PBSZCommand>()), "PBSZ <size>" }, },
    { "PROT",   { 1,  1,  ftp::ClientState::LoggedOut,
                  CreatorBasePtr(new Creator<rfc::PROTCommand>()), "PROT C|P" }, },
    { "PWD",    { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::PWDCommand>()), "PWD" }, },
    { "QUIT",   { 0,  0,  ftp::ClientState::AnyState,
                  CreatorBasePtr(new Creator<rfc::QUITCommand>()), "QUIT" }, },
    { "REIN",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "REST",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "RETR",   { 1,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::RETRCommand>()), "RETR <path>" }, },
    { "RMD",    { 1,  -1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::RMDCommand>()), "RMD <path>" }, },
    { "RNFR",   { 1,  -1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::RNFRCommand>()), "REFR <path>" }, },
    { "RNTO",   { 1,  -1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::RNTOCommand>()), "RNTO <path>" }, },
    { "SITE",   { 1,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::SITECommand>()), "STIE <command> [<arguments>..]" }, },
    { "SIZE",   { 1,  -1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::SIZECommand>()), "SIZE <path>" }, },
    { "SMNT",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "SSCN",   { 0,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::SSCNCommand>()), "SSCN [ON|OFF]" }, },
    { "STAT",   { 0,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::STATCommand>()), "STAT [[-<options] [<path>]]" }, },
    { "STOR",   { 1,  -1, ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::STORCommand>()), "STOR <path>" }, },
    { "STOU",   { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::STOUCommand>()), "STOU" }, },
    { "STRU",   { 0,  -1, ftp::ClientState::AnyState, nullptr, "NOT IMPLEMENTED" }, },
    { "SYST",   { 0,  0,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::SYSTCommand>()), "SYST" }, },
    { "TYPE",   { 1,  1,  ftp::ClientState::LoggedIn,
                  CreatorBasePtr(new Creator<rfc::TYPECommand>()), "TYPE I|A" }, },
    { "USER",   { 1, -1,  ftp::ClientState::LoggedOut,
                  CreatorBasePtr(new Creator<rfc::USERCommand>()), "USER <user>" }, }
  };
}

CommandDefOptRef Factory::Lookup(const std::string& command)
{
  CommandDefMap::const_iterator it = factory.defs.find(command);
  if (it != factory.defs.end()) return CommandDefOptRef(it->second);
  return CommandDefOptRef();
}

} /* rfc namespace */
} /* cmd namespace */

#ifdef CMD_RFC_FACTORY_TEST

#include <iostream>
#include <memory>
#include "ftp/client.hpp"

int main()
{
  using namespace cmd;
  
  ftp::ClientState::Client client;
  Args args;
  args.push_back("USER");
  std::unique_ptr<Command> cmd(Factory::Create(client, args));
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd.reset(Factory::Create(client, args));
  cmd->Execute();
}

#endif
