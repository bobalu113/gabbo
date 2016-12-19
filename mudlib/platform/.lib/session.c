/**
 * Utility library for tracking sessions.
 *
 * @author devo@eotl
 * @alias SessionLib
 */
#include <session.h>

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
  struct ConnectedSessionInfo *connections;
};

int is_active(string session_id) {
  return member(([ SESSION_STATE_RUNNING, SESSION_STATE_SUSPENDED ]), 
                SessionTracker->query_state(session_id)); 
}

int is_subsession(string session_id, string subsession_id) {
  mapping subsessions = SessionTracker->get_subsessions(session_id);
  if (subsessions) {
    return member(subsessions, subsession_id);
  }
  return 0;
}
