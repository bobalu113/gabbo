/**
 * The SessionTracker monitors user login sessions.
 * 
 * @alias SessionTracker
 */

private variables private functions inherit SessionLib;

// program_id#session_count
// ([ string session_id : SessionInfo session ])
mapping sessions;
// ([ userid : SessionInfo session ])
mapping last_sessions;
int session_counter;

string new_session(int user_id) {
  string id = generate_id();
  sessions[id] = (<SessionInfo>
    id: id,
    user_id: user_id,
    login_time: time()
  );
  last_sessions[user_id] = sessions[id];
  return id;
}

int session_ended(int user_id) {
  if (!member(last_sessions, user_id)) {
    return 0;
  }
  if (last_sessions[user_id]->logout_time != 0) {
    return 0;
  }
  last_sessions[user_id]->logout_time = time();
  return 1;
}

struct SessionInfo query_last_session(int user_id) {
  return last_sessions[user_id];
}

string generate_id() {
  return sprintf("%s#%d", 
                 ObjectTracker->query_object_id(THISO), ++session_counter);
}
