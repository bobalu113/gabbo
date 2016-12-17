/**
 * The ConnectionTracker monitors interactive connections to the game across
 * exec calls, and handles all telnet negotiations.
 * 
 * @alias ConnectionTracker
 */

#include <telnet.h>
#include <connection.h>

inherit SqlMixin;
inherit ConnectionLib;

#define NEGOTIATION_LOG

// ([ str connection_id : ConnectionState state ])
private mapping connections;
// ([ ob interactive : str connection_id ])
private mapping interactives;
private int connection_counter;

string query_connection(object interactive);
object query_interactive(string id);
int query_exec_time(string id);
struct ConnectionInfo query_connection_info(string id);
string generate_id();

#ifdef NEGOTIATION_LOG
void telnet_neg_log(string s) {
  object logger = LoggerFactory->get_logger(THISO);
  logger->debug("%O:%O:%s:%s:%s\n",
    previous_object(), 
    THISP,
    stringp(query_ip_name(THISP)) ? query_ip_name(THISP) : "NOHOST",
    stringp(query_ip_number(THISP)) ? query_ip_number(THISP) : "NOIP",
    s
  ); 
}
#endif

string new_connection(object interactive) {
  if (!interactive(interactive)) {
    raise_error(sprintf(
      "new_connection() of non-interactive: %O", 
      interactive
    ));
  }

  string connection_id = generate_id();
  connections[connection_id] = (<ConnectionState> 
    interactive: interactive,
    exec_time: time(),
    negotiation_pending: ([ ]),
    naws_last: 0,
    info: (<ConnectionInfo> id: 
      connection_id, 
      connect_time: time(),
    ),
    session: 0
  );
  interactives[interactive] = connection_id;
  return connection_id;
}

int set_interactive(string connection_id, object interactive) {
  if (!interactive(interactive)) {
    return 0;
  }
  if (!member(connections, connection_id)) {
    return 0;
  }
  connections[connection_id]->interactive = interactive;
  connections[connection_id]->exec_time = time();
  return 1;
}

int set_session(string connection_id, string session_id) {
  if (!member(connections, connection_id)) {
    return 0;
  }
  connections[connection_id]->session = session_id;
  return 1;
}

string query_connection(object interactive) {
  if (!member(interactives, interactive)) {
    return 0;
  }
  return interactives[interactive];
}

object query_interactive(string connection_id) {
  if (!member(connections, connection_id)) {
    return 0;
  }
  return connections[connection_id]->interactive;
}

int query_exec_time(string connection_id) {
  if (!member(connections, connection_id)) {
    return 0;
  }
  return connections[connection_id]->exec_time;
}

void telnet_negotiation(int cmd, int opt, int *optargs) {
  if (previous_object() != THISP) {
    return;
  }

  string connection_id = query_connection(THISP);
  if (!connection_id) {
    // I hope this should never happen
    connection_id = new_connection(THISP);
  }
  struct ConnectionState connection = connections[connection_id];
  object interactive = connection->interactive;
  struct ConnectionInfo info = connection->info;

  int dont_log;

#ifdef NEGOTIATION_LOG
  string output, command, option, option_arguments;

  if (!TELCMD_OK(cmd)) {
    command = "<bad>";
  } else {
    command = TELCMD(cmd);
  }
  if (!TELOPT_OK(opt)) {
    option = "<bad>";
  } else {
    option = TELOPT(opt);
  }
  if (!optargs) {
    option_arguments = "";
  } else {
    if (optargs[0] == TELQUAL_IS) {
      option_arguments = "IS";
    } else {
      if (optargs[0] == TELQUAL_SEND) {
        option_arguments = "SEND";
      }
    }
  }
#endif
  switch (cmd) {
    case WILL:
      if (!connection->negotiation_pending[opt]) {
        send_binary_message(interactive, ({ IAC, DO, opt }));
      }
      connection->negotiation_pending[opt] = 0;
      send_binary_message(interactive, ({ IAC, SB, opt, TELQUAL_SEND, IAC, SE }));
      break;
    case DO:
      send_binary_message(interactive, ({ IAC, WILL, opt }));
      break;
    case SB:
      if (opt == TELOPT_TTYPE && optargs[0] == TELQUAL_IS) {
        info->terminal = to_string(optargs[1..]);
#ifdef NEGOTIATION_LOG
        option_arguments += " " + info->terminal;
#endif
        break;
      }
      if (opt == TELOPT_NAWS && sizeof(optargs) >= 4) {
        info->terminal_width = optargs[1] - optargs[0];
        info->terminal_height = optargs[3] - optargs[2];
#ifdef NEGOTIATION_LOG
        option_arguments = sprintf("%d %d %d %d",
                                   optargs[0], optargs[1],
                                   optargs[2], optargs[3]);
#endif
        break;
      }
      if (opt == TELOPT_TTYLOC 
          && optargs[0] == TELQUAL_IS 
          && sizeof(optargs) > 8) { 
        if (info->ttyloc == 0) {
          info->ttyloc = (int *) allocate(8);
        }
        for (int i = 0; i < 8; i++) {
          info->ttyloc[i] = optargs[i + 1];
        }
#ifdef NEGOTIATION_LOG
        option_arguments = sprintf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                                   info->ttyloc[0], info->ttyloc[1],
                                   info->ttyloc[2], info->ttyloc[3],
                                   info->ttyloc[4], info->ttyloc[5],
                                   info->ttyloc[6], info->ttyloc[7]);
#endif
        break;
      } 
      connection->negotiation_pending[opt] = 0;
      break;
  }
#ifdef NEGOTIATION_LOG
  if (opt == TELOPT_NAWS && ((connection->naws_last + 30) > time())) {
    dont_log = 1;
  }
  if (opt == TELOPT_NAWS && cmd == SB && !dont_log) {
     connection->naws_last = time();
  } 

  if (!dont_log) {
     output = sprintf("IAC %s %s %s",
                      command, option, option_arguments);
     telnet_neg_log(output);
   }
#endif
}

int telnet_get_terminal(object interactive) {
  string connection_id = query_connection(interactive);
  if (!connection_id) {
    return 0;
  }
  struct ConnectionState connection = connections[connection_id];
  struct ConnectionInfo info = connection->info;

#ifdef NEGOTIATION_LOG
  telnet_neg_log("telnet_get_terminal");
#endif
  send_binary_message(interactive, ({ IAC, DO, TELOPT_TTYPE }));
  connection->negotiation_pending += ([ TELOPT_TTYPE : 1 ]);
  return 1;
}

int telnet_get_NAWS(object interactive) {
  string connection_id = query_connection(interactive);
  if (!connection_id) {
    return 0;
  }
  struct ConnectionState connection = connections[connection_id];
  struct ConnectionInfo info = connection->info;
#ifdef NEGOTIATION_LOG
  telnet_neg_log("telnet_get_NAWS");
#endif
  send_binary_message(interactive, ({ IAC, DO, TELOPT_NAWS }));
  connection->negotiation_pending += ([ TELOPT_NAWS : 1 ]);
  return 1;
}

int telnet_get_ttyloc(object interactive) {
  string connection_id = query_connection(interactive);
  if (!connection_id) {
    return 0;
  }
  struct ConnectionState connection = connections[connection_id];
  struct ConnectionInfo info = connection->info;
#ifdef NEGOTIATION_LOG
  telnet_neg_log("telnet_get_ttyloc");
#endif
  send_binary_message(interactive, ({ IAC, DO, TELOPT_TTYLOC }));
  connection->negotiation_pending += ([ TELOPT_TTYLOC : 1 ]);
  return 1;
}

string generate_id() {
  return sprintf("%s#%d", 
                 ObjectTracker->query_object_id(THISO), ++connection_counter);
}

void create() {
  connections = ([ ]);
  interactives = ([ ]);
  connection_counter = 0;
  return;
}
