#include <String.hh>
#include "ChServer.hh"
#include "semantics.hh"

ChServer::ChServer(const Slot &args)
  : Server(args[0]),
    command_interpreter(args[1]),
    connect_command((args->length() > 2)?args[2]:Slot("incoming"))
{
}

Slot ChServer::incoming(const Slot from)
{
  Chaos *c = dynamic_cast<Chaos*>((Data*)command_interpreter);
  c->ChPush(from);
  c->ChPush(connect_command);
  c->ChEval();
  return this;
}

