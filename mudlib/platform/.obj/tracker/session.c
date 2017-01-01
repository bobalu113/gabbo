/**
 * The SessionTracker monitors user login sessions.
 * 
 * @alias SessionTracker
 */
#include <session.h>

inherit SessionLib;

// program_id#session_count
// ([ string session_id : SessionInfo session ])
mapping sessions;
int session_counter;

string new_session(string user_id, string supersession_id);
string query_user(string session_id);
int is_connected(string session_id);
int connect_session(string session_id, string connection);
int disconnect_session(string session_id);
string generate_id();

string new_session(string user_id, string supersession_id) {
  object logger = LoggerFactory->get_logger(THISO);
  if (supersession_id && !member(sessions, supersession_id)) {
    logger->warn("unable to create session: supersession %O doesn't exist",
                 supersession_id);
    return 0;
  }
  string id = generate_id();
  sessions[id] = (<SessionInfo>
    id: id,
    user: user_id,
    state: SESSION_STATE_NEW,
    create_time: time(),
    subsessions: ([ ]),
    supersessions: ([ ]),
    connections: ({ })
  );
  if (supersession_id) {
    sessions[id]->supersessions += ([ supersession_id ]);
    sessions[supersession_id]->subsessions += ([ id ]);
  }
  return id;
}

string query_user(string session_id) {
  if (member(sessions, session_id)) {
    return sessions[session_id]->user;
  }
  return 0;
}

int is_connected(string session_id) {
  if (!member(sessions, session_id)) {
    return 0;
  }
  struct SessionInfo session = sessions[session_id];
  return (sizeof(session->connections) 
          && (session->connections[<1]->disconnect_time));
}

int connect_session(string session_id, string connection) {
  if (!member(sessions, session_id)) {
    return 0;
  }
  disconnect_session(session_id);
  struct SessionInfo session = sessions[session_id];
  session->connections += ({ (<ConnectedSessionInfo>
    connection: connection,
    connect_time: time()
  ) });
  return 1;
}

int disconnect_session(string session_id) {
  if (!member(sessions, session_id)) {
    return 0;
  }
  if (!is_connected(session_id)) {
    return 0;
  }
  struct SessionInfo session = sessions[session_id];
  session->connections[<1]->disconnect_time = time();
  return 1;
}

int resume_session(string session_id) {
  if (!member(sessions, session_id)) {
    return 0;
  }
  struct SessionInfo session = sessions[session_id];
  int previous = session->state;
  if (previous == SESSION_STATE_DONE) {
    return 0;
  }
  session->state = SESSION_STATE_RUNNING;
  return previous;
}

int set_avatar(string session_id, object avatar) {
  if (!member(sessions, session_id)) {
    return 0;
  }
  sessions[session_id]->avatar = avatar;
  return 1;
}

object query_avatar(string session_id) {
  if (!member(sessions, session_id)) {
    return 0;
  }
  return sessions[session_id]->avatar;
}

string generate_id() {
  return sprintf("%s#%d", 
                 ObjectTracker->query_object_id(THISO), ++session_counter);
}

int create() {
  sessions = ([ ]);
  return 0;
}