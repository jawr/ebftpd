#include "factory.hpp"
#include "cmd/command.hpp"

namespace cmd
{

Factory Factory::factory;

Factory::Factory()
{
  Register("ABOR", new Creator<ABORCommand>());
  Register("ACCT", new Creator<ACCTCommand>());
  Register("ADAT", new Creator<ADATCommand>());
  Register("ALLO", new Creator<ALLOCommand>());
  Register("APPE", new Creator<APPECommand>());
  Register("AUTH", new Creator<AUTHCommand>());
  Register("CCC", new Creator<CCCCommand>());
  Register("CDUP", new Creator<CDUPCommand>());
  Register("CONF", new Creator<CONFCommand>());
  Register("CWD", new Creator<CWDCommand>());
  Register("DELE", new Creator<DELECommand>());
  Register("ENC", new Creator<ENCCommand>());
  Register("EPRT", new Creator<EPRTCommand>());
  Register("EPSV", new Creator<EPSVCommand>());
  Register("FEAT", new Creator<FEATCommand>());
  Register("HELP", new Creator<HELPCommand>());
  Register("LANG", new Creator<LANGCommand>());
  Register("LIST", new Creator<LISTCommand>());
  Register("LPRT", new Creator<LPRTCommand>());
  Register("LPSV", new Creator<LPSVCommand>());
  Register("MDTM", new Creator<MDTMCommand>());
  Register("MIC", new Creator<MICCommand>());
  Register("MKD", new Creator<MKDCommand>());
  Register("MLSD", new Creator<MLSDCommand>());
  Register("MLST", new Creator<MLSTCommand>());
  Register("MODE", new Creator<MODECommand>());
  Register("NLST", new Creator<NLSTCommand>());
  Register("NOOP", new Creator<NOOPCommand>());
  Register("OPTS", new Creator<OPTSCommand>());
  Register("PASS", new Creator<PASSCommand>());
  Register("PASV", new Creator<PASVCommand>());
  Register("PBSZ", new Creator<PBSZCommand>());
  Register("PORT", new Creator<PORTCommand>());
  Register("PROT", new Creator<PROTCommand>());
  Register("PWD", new Creator<PWDCommand>());
  Register("QUIT", new Creator<QUITCommand>());
  Register("REIN", new Creator<REINCommand>());
  Register("REST", new Creator<RESTCommand>());
  Register("RETR", new Creator<RETRCommand>());
  Register("RMD", new Creator<RMDCommand>());
  Register("RNFR", new Creator<RNFRCommand>());
  Register("RNTO", new Creator<RNTOCommand>());
  Register("SITE", new Creator<SITECommand>());
  Register("SIZE", new Creator<SIZECommand>());
  Register("SMNT", new Creator<SMNTCommand>());
  Register("STAT", new Creator<STATCommand>());
  Register("STOR", new Creator<STORCommand>());
  Register("STOU", new Creator<STOUCommand>());
  Register("STRU", new Creator<STRUCommand>());
  Register("SYST", new Creator<SYSTCommand>());
  Register("TYPE", new Creator<TYPECommand>());
  Register("USER", new Creator<USERCommand>());
  Register("XCUP", new Creator<XCUPCommand>());
  Register("XMKD", new Creator<XMKDCommand>());
  Register("XPWD", new Creator<XPWDCommand>());
  Register("XRCP", new Creator<XRCPCommand>());
  Register("XRMD", new Creator<XRMDCommand>());
  Register("XRSQ", new Creator<XRSQCommand>());
  Register("XSEM", new Creator<XSEMCommand>());
  Register("XSEN", new Creator<XSENCommand>());
}

Factory::~Factory()
{
  while (!creators.empty())
  {
    delete creators.begin()->second;
    creators.erase(creators.begin());
  }
}

void Factory::Register(const std::string& command,
                       CreatorBase<Command>* creator)
{
  creators.insert(std::make_pair(command, creator));
}  

Command* Factory::Create(ftp::Client& client, const Args& args)
{
  typename CreatorsMap::const_iterator it = factory.creators.find(args[0]);
  if (it == factory.creators.end()) return 0;
  return it->second->Create(client, args);
}


} /* cmd namespace */

#ifdef CMD_FACTORY_TEST

#include <iostream>
#include <memory>
#include "ftp/client.hpp"

int main()
{
  using namespace cmd;
  
  ftp::Client client;
  Args args;
  args.push_back("USER");
  std::auto_ptr<Command> cmd(Factory::Create(client, args));
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd.reset(Factory::Create(client, args));
  cmd->Execute();
}

#endif
