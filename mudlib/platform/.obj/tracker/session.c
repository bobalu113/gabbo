/**
 * The SessionTracker monitors user login sessions.
 * 
 * @alias SessionTracker
 */

inherit SessionLib;

// program_id#session_count
// ([ string session_id : SessionInfo session ])
mapping sessions;
// ([ user_id : SessionInfo session ])
mapping last_sessions;
int session_counter;
string generate_id();

string new_session(string user_id, string supersession_id) {
  if (supersession_id && !member(sessions, supersession_id)) {
    logger->warn("unable to create session: supersession %O doesn't exist",
                 supersession_id);
    return 0;
  }
  string id = generate_id();
  sessions[id] = (<SessionInfo>
    id: id,
    user_id: user_id,
    create_time: time(),
    subsessions: ([ ]),
    supersessions: ([ ])
  );
  if (supersession_id) {
    sessions[id]->supersessions += ([ supersession_id ]);
    sessions[supersession_id] += ([ id ]);
  }
  last_sessions[user_id] = sessions[id];
  return id;
}

string query_user(string session_id) {
  if (member(sessions, session_id)) {
    return sessions[session_id]->user;
  }
  return 0;
}

int session_ended(string user_id) {
  if (!member(last_sessions, user_id)) {
    return 0;
  }
  if (last_sessions[user_id]->logout_time != 0) {
    return 0;
  }
  last_sessions[user_id]->logout_time = time();
  return 1;
}

struct SessionInfo query_last_session(string user_id) {
  return last_sessions[user_id];
}

string generate_id() {
  return sprintf("%s#%d", 
                 ObjectTracker->query_object_id(THISO), ++session_counter);
}
