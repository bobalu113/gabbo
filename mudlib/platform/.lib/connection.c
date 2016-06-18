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

int switch_connection(object from, object to) {
  object logger = LoggerFactory->get_logger(THISO);
  if (!exec(to, from)) {
    logger->debug("exec failed: %O %O", to, from);
    return 0;
  }
  if (!ConnectionTracker->switch_connection(from, to)) {
    logger->debug("failed to switch connection: %O %O", from, to);
    return 0;
  }
  return 1;
}
