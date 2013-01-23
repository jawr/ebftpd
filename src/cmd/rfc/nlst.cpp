#include "cmd/rfc/list.hpp"
#include "cmd/rfc/nlst.hpp"


namespace cmd { namespace rfc
{

void NLSTCommand::Execute()
{
  LISTCommand(client, argStr, args).ExecuteNLST();
}

} /* rfc namespace */
} /* cmd namespace */
