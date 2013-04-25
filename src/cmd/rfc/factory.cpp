//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "cmd/rfc/factory.hpp"
#include "cmd/rfc/commands.hpp"
#include "cmd/rfc/cwd.hpp"
#include "cmd/rfc/pass.hpp"
#include "cmd/rfc/retr.hpp"
#include "cmd/rfc/stor.hpp"
#include "cmd/rfc/dirlist.hpp"

namespace cmd { namespace rfc
{

std::unique_ptr<Factory> Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "ABOR",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<ABORCommand>>(), "SITE ABOR" }, },
    { "ACCT",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "ADAT",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "ALLO",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "APPE",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "AUTH",   { 1,  1,  ftp::ClientState::LoggedOut,        ftp::ActionNotOkay,
                  std::make_shared<Creator<AUTHCommand>>(), "SITE AUTH TLS|SSL" }, },
    { "CCC",    { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "CDUP",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<CDUPCommand>>(), "CDUP" }, },
    { "CPSV",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<CPSVCommand>>(), "CPSV" }, },
    { "CWD",    { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  std::make_shared<Creator<CWDCommand>>(), "CWD <path>" }, },
    { "DELE",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  std::make_shared<Creator<DELECommand>>(), "DELE <path>" }, },
    { "ENC",    { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "EPRT",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<EPRTCommand>>(), "EPRT <port-string>" }, },
    { "EPSV",   { 0,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<EPSVCommand>>(), "EPSV [MODE|EXTENDED|NORMAL]" }, },
    { "FEAT",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<FEATCommand>>(), "FEAT" }, },
    { "HELP",   { 0,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<HELPCommand>>(), "HELP [<command>]" }, },
    { "LANG",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "LIST",   { 0,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<LISTCommand>>(), "LIST [-<options>] [<path>]" }, },
    { "LPRT",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<LPRTCommand>>(), "LPRT <port-string>" }, },
    { "LPSV",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<LPSVCommand>>(), "LPSV" }, },
    { "MDTM",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<MDTMCommand>>(), "MDTM <path>" }, },
    { "MFF",    { 1,  -1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  nullptr,    "MFF <fact> [ ; <fact> ..] <path>" }, },
    { "MFCT",   { 2,  2,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  nullptr,   "MFCT <time-val> <path>" }, },
    { "MFMT",   { 2,  2,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<MFMTCommand>>(),   "MFMT <time-val> <path>" }, },
    { "MIC",    { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "MKD",    { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<MKDCommand>>(), "MKD <path>" }, },
    { "MLSD",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "MLST",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "MODE",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<MODECommand>>(), "MODE S|B|C" }, },
    { "NLST",   { 0,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<NLSTCommand>>(), "NLST [-<options>] [<path>]" }, },
    { "NOOP",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<NOOPCommand>>(), "NOOP" }, },
    { "OPTS",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "PASS",   { 0,  -1, ftp::ClientState::WaitingPassword,  ftp::ActionNotOkay,
                  std::make_shared<Creator<PASSCommand>>(), "PASS <password>" }, },
    { "PASV",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<PASVCommand>>(), "PASV" }, },
    { "PORT",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<PORTCommand>>(), "PORT <port-string>" }, },
    { "PBSZ",   { 1,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<PBSZCommand>>(), "PBSZ <size>" }, },
    { "PROT",   { 1,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<PROTCommand>>(), "PROT C|P" }, },
    { "PWD",    { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<PWDCommand>>(), "PWD" }, },
    { "QUIT",   { 0,  0,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<QUITCommand>>(), "QUIT" }, },
    { "REIN",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "REST",   { 1,  1,  ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  std::make_shared<Creator<RESTCommand>>(), "REST <offset>" }, },
    { "RETR",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<RETRCommand>>(), "RETR <path>" }, },
    { "RMD",    { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  std::make_shared<Creator<RMDCommand>>(), "RMD <path>" }, },
    { "RNFR",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  std::make_shared<Creator<RNFRCommand>>(), "REFR <path>" }, },
    { "RNTO",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  std::make_shared<Creator<RNTOCommand>>(), "RNTO <path>" }, },
    { "SITE",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<SITECommand>>(), "STIE <command> [<arguments>..]" }, },
    { "SIZE",   { 1,  -1,  ftp::ClientState::LoggedIn,        ftp::ActionNotOkay,
                  std::make_shared<Creator<SIZECommand>>(), "SIZE <path>" }, },
    { "SMNT",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "SSCN",   { 0,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<SSCNCommand>>(), "SSCN [ON|OFF]" }, },
    { "STAT",   { 0,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<STATCommand>>(), "STAT [[-<options] [<path>]]" }, },
    { "STOR",   { 1,  -1, ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<STORCommand>>(), "STOR <path>" }, },
    { "STOU",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<STOUCommand>>(), "STOU" }, },
    { "STRU",   { 0,  -1, ftp::ClientState::AnyState,         ftp::ActionNotOkay,
                  nullptr, "NOT IMPLEMENTED" }, },
    { "SYST",   { 0,  0,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<SYSTCommand>>(), "SYST" }, },
    { "TYPE",   { 1,  1,  ftp::ClientState::LoggedIn,         ftp::ActionNotOkay,
                  std::make_shared<Creator<TYPECommand>>(), "TYPE I|A" }, },
    { "USER",   { 1, -1,  ftp::ClientState::LoggedOut,        ftp::ActionNotOkay,
                  std::make_shared<Creator<USERCommand>>(), "USER <user>" }, }
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
