/**
 * Utility library for tracking sessions.
 *
 * @author devo@eotl
 * @alias SessionLib
 */

struct ConnectedSessionInfo {
  string connection;
  int connect_time;
  int disconnect_time;
};

struct SessionInfo {
  string id;
  object avatar;
  string user;
  int state; // new, running, suspended, done
  int create_time;
  int transition_time;
  int invisible;
  mapping subsessions;
  mapping supersessions;
  struct ConnectedSessionInfo *connection_history;
};
