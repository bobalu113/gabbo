/**
 * The ConnectionTracker monitors interactive connections to the game across
 * exec calls, and handles all telnet negotiations.
 * 
 * @alias ConnectionTracker
 */

#include <telnet.h>
#include <connection.h>

private variables private functions inherit ConnectionLib;

#define NEGOTIATION_LOG

// ([ str connection_id : ({ ob interactive, 
//                           int exec_time,
//                           map negotiation_pending,
//                           int naws_last,
//                           ConnectionConfig config }) ])
private mapping connections;
// ([ ob interactive : str connection_id ])
private mapping interactives;

private int connection_count;

string query_connection_id(object interactive);
object query_interactive(string id);
int query_exec_time(string id);
struct ConnectionConfig query_config(string id);
string increment_connection_id();

#ifdef NEGOTIATION_LOG
void telnet_neg_log (string s) {
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

  string connection_id = increment_connection_id();
  connections[connection_id] = ({ 
    interactive,
    time(),
    ([ ]),
    0,
    (<ConnectionConfig> id: connection_id, connect_time: time())
  });
  interactives[interactive] = connection_id;
  return connection_id;
}

int switch_connection(mixed connection, object interactive) {
  if (!interactive(interactive)) {
    return 0;
  }
  if (objectp(connection) && interactives[connection]) {
    connection = connections[interactives[connection]];
  } else if (member(connections, connection)) {
    connection = connections[connection];
  } else {
    return 0;
  }
  connection[CON_INTERACTIVE] = interactive;
  connection[CON_EXEC_TIME] = time();
  return 1;
}

void telnet_negotiation(int cmd, int opt, int *optargs) {
  if (previous_object() != THISP) {
    return;
  }

  string connection_id = query_connection_id(THISP);
  if (!connection_id) {
    // I hope this should never happen
    connection_id = new_connection(THISP);
  }
  mixed *connection = connections[connection_id];
  object interactive = connection[CON_INTERACTIVE];
  struct ConnectionConfig config = connection[CON_CONFIG];

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
      if (!connection[CON_NEG_PEND][opt]) {
        send_binary_message(interactive, ({ IAC, DO, opt }));
      }
      connection[CON_NEG_PEND][opt] = 0;
      send_binary_message(interactive, ({ IAC, SB, opt, TELQUAL_SEND, IAC, SE }));
      break;
    case DO:
      send_binary_message(interactive, ({ IAC, WILL, opt }));
      break;
    case SB:
      if (opt == TELOPT_TTYPE && optargs[0] == TELQUAL_IS) {
        config->terminal = to_string(optargs[1..]);
#ifdef NEGOTIATION_LOG
        option_arguments += " " + config->terminal;
#endif
        break;
      }
      if (opt == TELOPT_NAWS && sizeof(optargs) >= 4) {
        config->terminal_width = optargs[1] - optargs[0];
        config->terminal_height = optargs[3] - optargs[2];
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
        if (config->ttyloc == 0) {
          config->ttyloc = (int *) allocate(8);
        }
        for (int i = 0; i < 8; i++) {
          config->ttyloc[i] = optargs[i + 1];
        }
#ifdef NEGOTIATION_LOG
        option_arguments = sprintf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                                   config->ttyloc[0], config->ttyloc[1],
                                   config->ttyloc[2], config->ttyloc[3],
                                   config->ttyloc[4], config->ttyloc[5],
                                   config->ttyloc[6], config->ttyloc[7]);
#endif
        break;
      } 
      connection[CON_NEG_PEND][opt] = 0;
      break;
  }
#ifdef NEGOTIATION_LOG
  if (opt == TELOPT_NAWS && ((connection[CON_NAWS_LAST] + 30) > time())) {
    dont_log = 1;
  }
  if (opt == TELOPT_NAWS && cmd == SB && !dont_log) {
     connection[CON_NAWS_LAST] = time();
  } 

  if (!dont_log) {
     output = sprintf("IAC %s %s %s",
                      command, option, option_arguments);
     telnet_neg_log(output);
   }
#endif
}

int telnet_get_terminal(object interactive) {
  string connection_id = query_connection_id(interactive);
  if (!connection_id) {
    return 0;
  }
  mixed *connection = connections[connection_id];
  struct ConnectionConfig config = connection[CON_CONFIG];

#ifdef NEGOTIATION_LOG
  telnet_neg_log("telnet_get_terminal");
#endif
  send_binary_message(interactive, ({ IAC, DO, TELOPT_TTYPE }));
  connection[CON_NEG_PEND] += ([ TELOPT_TTYPE : 1 ]);
  return 1;
}

int telnet_get_NAWS(object interactive) {
  string connection_id = query_connection_id(interactive);
  if (!connection_id) {
    return 0;
  }
  mixed *connection = connections[connection_id];
  struct ConnectionConfig config = connection[CON_CONFIG];
#ifdef NEGOTIATION_LOG
  telnet_neg_log("telnet_get_NAWS");
#endif
  send_binary_message(interactive, ({ IAC, DO, TELOPT_NAWS }));
  connection[CON_NEG_PEND] += ([ TELOPT_NAWS : 1 ]);
  return 1;
}

int telnet_get_ttyloc(object interactive) {
  string connection_id = query_connection_id(interactive);
  if (!connection_id) {
    return 0;
  }
  mixed *connection = connections[connection_id];
  struct ConnectionConfig config = connection[CON_CONFIG];
#ifdef NEGOTIATION_LOG
  telnet_neg_log("telnet_get_ttyloc");
#endif
  send_binary_message(interactive, ({ IAC, DO, TELOPT_TTYLOC }));
  connection[CON_NEG_PEND] += ([ TELOPT_TTYLOC : 1 ]);
  return 1;
}

string query_connection_id(object interactive) {
  return (member(interactives, interactive) && interactives[interactive]);
}

struct ConnectionConfig query_config(string id) {
  return (member(connections, id) && connections[id][CON_CONFIG]);
}

object query_interactive(string id) {
  return (member(connections, id) && connections[id][CON_INTERACTIVE]);
}

int query_exec_time(string id) {
  return (member(connections, id) && connections[id][CON_EXEC_TIME]);
}

string increment_connection_id() {
  return "connection#" + ++connection_count;
}

void create() {
  connections = ([ ]);
  interactives = ([ ]);
  connection_count = 0;
  return;
}