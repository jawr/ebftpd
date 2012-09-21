#ifndef __CMD_FACTORY_HPP
#define __CMD_FACTORY_HPP

#include <tr1/memory>

namespace cmd
{

<template class CommandT>
class Factory
{
public:
  Command* Create(Client& client, const Args& args)
  {
    return new CommandT(client, args);
  }
};

class UserFactory
{
public:
  UserCommand* Create
};

} /* cmd namespace */

#endif
