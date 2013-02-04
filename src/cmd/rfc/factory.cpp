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
#include "cmd/rfc/cpsv.hpp"
#include "cmd/rfc/rest.hpp"

namespace cmd { namespace rfc
{

std::unique_ptr<Factory> Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "ABOR",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<ABORCommand>()), "SITE ABOR" }, },
    { "ACCT",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "ADAT",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "ALLO",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "APPE",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "AUTH",   { 1,  1,  ftp::ClientState::LoggedOut,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<AUTHCommand>()), "SITE AUTH TLS|SSL" }, },
    { "CCC",    { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "CDUP",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<CDUPCommand>()), "CDUP" }, },
    { "CPSV",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<CPSVCommand>()), "CPSV" }, },
    { "CWD",    { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<CWDCommand>()), "CWD <path>" }, },
    { "DELE",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<DELECommand>()), "DELE <path>" }, },
    { "ENC",    { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "EPRT",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<EPRTCommand>()), "EPRT <port-string>" }, },
    { "EPSV",   { 0,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<EPSVCommand>()), "EPSV [MODE|EXTENDED|NORMAL]" }, },
    { "FEAT",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<FEATCommand>()), "FEAT" }, },
    { "HELP",   { 0,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<HELPCommand>()), "HELP [<command>]" }, },
    { "LANG",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "LIST",   { 0,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<LISTCommand>()), "LIST [-<options>] [<path>]" }, },
    { "LPRT",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<LPRTCommand>()), "LPRT <port-string>" }, },
    { "LPSV",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<LPSVCommand>()), "LPSV" }, },
    { "MDTM",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<MDTMCommand>()), "MDTM <path>" }, },
    { "MIC",    { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "MKD",    { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<MKDCommand>()), "MKD <path>" }, },
    { "MLSD",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "MLST",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "MODE",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<MODECommand>()), "MODE S|B|C" }, },
    { "NLST",   { 0,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<NLSTCommand>()), "NLST [-<options>] [<path>]" }, },
    { "NOOP",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<NOOPCommand>()), "NOOP" }, },
    { "OPTS",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "PASS",   { 0,  -1, ftp::ClientState::WaitingPassword,  ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<PASSCommand>()), "PASS <password>" }, },
    { "PASV",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<PASVCommand>()), "PASV" }, },
    { "PORT",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<PORTCommand>()), "PORT <port-string>" }, },
    { "PBSZ",   { 1,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<PBSZCommand>()), "PBSZ <size>" }, },
    { "PROT",   { 1,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<PROTCommand>()), "PROT C|P" }, },
    { "PWD",    { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<PWDCommand>()), "PWD" }, },
    { "QUIT",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<QUITCommand>()), "QUIT" }, },
    { "REIN",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "REST",   { 1,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<RESTCommand>()), "REST <offset>" }, },
    { "RETR",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<RETRCommand>()), "RETR <path>" }, },
    { "RMD",    { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<RMDCommand>()), "RMD <path>" }, },
    { "RNFR",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<RNFRCommand>()), "REFR <path>" }, },
    { "RNTO",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<RNTOCommand>()), "RNTO <path>" }, },
    { "SITE",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<SITECommand>()), "STIE <command> [<arguments>..]" }, },
    { "SIZE",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<SIZECommand>()), "SIZE <path>" }, },
    { "SMNT",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "SSCN",   { 0,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<SSCNCommand>()), "SSCN [ON|OFF]" }, },
    { "STAT",   { 0,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<STATCommand>()), "STAT [[-<options] [<path>]]" }, },
    { "STOR",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<STORCommand>()), "STOR <path>" }, },
    { "STOU",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<STOUCommand>()), "STOU" }, },
    { "STRU",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "SYST",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<SYSTCommand>()), "SYST" }, },
    { "TYPE",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<TYPECommand>()), "TYPE I|A" }, },
    { "USER",   { 1, -1,  ftp::ClientState::LoggedOut,        ftp::ActionNotOkay,
                  CreatorBasePtr(new Creator<USERCommand>()), "USER <user>" }, }
  };
}

CommandDefOptRef Factory::Lookup(const std::string& command)
{
  CommandDefMap::const_iterator it = factory->defs.find(command);
  if (it != factory->defs.end()) return CommandDefOptRef(it->second);
  return CommandDefOptRef();
}

} /* rfc namespace */
} /* cmd namespace */
