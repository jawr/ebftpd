#include "cmd/rfc/help.hpp"
#include "cmd/rfc/factory.hpp"

namespace cmd { namespace rfc
{

void HELPCommand::Execute()
{
  if (args.size() == 2)
  {
    boost::to_upper(args[1]);
    CommandDefOptRef def(Factory::Lookup(args[1]));
    if (!def) control.Reply(ftp::CommandUnrecognised, "Command not understood");
    else control.Reply(ftp::CommandOkay, "Syntax: " + def->Syntax());
    return;
  }

  static const char* reply =
    " ebftpd Command listing:\n"
    "------------------------------------------------------------------\n"
    " ABOR *ACCT *ADAT *ALLO  APPE  AUTH *CCC   CDUP *CONF  CWD   DELE\n"
    "*ENC   EPRT  EPSV  FEAT  HELP *LANG  LIST *LPRT *LPSV  MDTM *MIC\n"
    " MKD  *MLSD *MLST  MODE  NLST  NOOP *OPTS  PASS  PASV  PBSZ  PORT\n"
    " PROT  PWD   QUIT *REIN *REST  RETR  RMD   RNFR  RNTO  SITE  SIZE\n"
    "*SMNT  STAT  STOR  STOU *STRU  SYST  TYPE\n"
    "------------------------------------------------------------------\n"
    "End of list.                         (* Commands not implemented)";
    
  control.Reply(ftp::HelpMessage, reply);
  return;
}

} /* rfc namespace */
} /* cmd namespace */
