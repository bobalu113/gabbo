/**
 * The UserTracker keeps track of the game's users.
 *
 * @author devo@eotl
 * @alias UserTracker
 */

inherit UserLib;

// ([ str user_id : UserInfo ])
mapping users;
// ([ str username : user_id ])
mapping primary_user_ids;
int user_counter;

string new_user(string username) {
  string user_id = generate_id();
  users[user_id] = (<UserInfo> 
    id: user_id,
    username: username
  );
  primary_user_ids[username] = user_id;
  return user_id;
}

string query_username(string user_id) {
  if (!member(users, user_id)) {
    return 0;
  }
  return users[user_id]->username;
}

int set_last_session(string user_id, string session_id) {
  if (!member(users, user_id)) {
    return 0;
  }
  users[user_id]->last_session = session_id;
  return 1;
}

string query_last_session(string user_id) {
  if (!member(users, user_id)) {
    return 0;
  }
  return users[user_id]->last_session;
}

string query_primary_user(string username) {
  return primary_user_ids[username];
}

string generate_id() {
  return sprintf("%s#%d", program_name(THISO), ++user_counter);
}