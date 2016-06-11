/**
 * Utility library for tracking connections.
 *
 * @author devo@eotl
 * @alias ConnectionLib
 */

struct ConnectionInfo {
  string id;
  string terminal;
  int terminal_width, terminal_height;
  int *ttyloc;
  int connect_time;
  int disconnect_time;
};

struct ConnectionState {
  object interactive;
  int exec_time;
  mapping negotiation_pending;
  int naws_last;
  struct ConnectionInfo info;
  string *sessions;
};
