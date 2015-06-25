#include <Server.hh>
class ChServer
  : public Server
{
protected:
  Slot command_interpreter;
  Slot connect_command;
public:
  /** Construct a ChServer
   * @param args[0] port
   * @param args[1] command interpreter to pass command to
   * @param args[2] connection command (default 'incoming')
   */
  ChServer(const Slot &args);
  virtual Slot incoming(const Slot from);
};

