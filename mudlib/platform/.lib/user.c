/**
 * Utility library for dealing with users.
 * 
 * @author devo@eotl
 * @alias UserLib
 */
#include <user.h>

inherit FileLib;
inherit ConnectionLib;
inherit SessionLib;

struct UserInfo {
  string id;
  string username;
  string last_session;
};

/**
 * Get the user directory for a given username.
 * 
 * @param  username the username of directory owner
 * @return          the user dir
 */
string user_dir(string username) {
  return UserDir "/" + username;
}

/**
 * Test whether a user with a given name exists.
 * 
 * @param  username the username to test
 * @return          1 if the user exists, otherwise 0
 */
int user_exists(string username) {
  return file_exists(user_dir(username));
}

string passwd_file(string username) {
  return user_dir(username) + PASSWD_FILE;
}

string hash_passwd(string password) {
  return hash(PASSWD_HASH_METHOD, password, PASSWD_HASH_ITERATIONS);
}

string create_user(string username, string password) {
  object logger = LoggerFactory->get_logger(THISO);
  string user_dir = user_dir(username);
  // right now only one user id per user name/dir, but might change someday
  if (file_exists(user_dir)) {
    logger->warn("user directory already exists: %O", user_dir);
    return 0;
  } else {
    copy_tree(SkelDir, user_dir);
  }

  // create user
  string user_id = UserTracker->new_user(username);

  // save password
  mapping passwd = read_value(passwd_file(username));
  if (!mappingp(passwd)) {
    passwd = ([ ]);
  }
  string hash = hash_passwd(password);
  passwd[user_id] = ([ PASSWD_PASSWORD: hash ]);
  write_value(passwd_file(username), passwd);
  
  return user_id;
}

string attach_session(object login, string user_id) {
  object logger = LoggerFactory->get_logger(THISO);

  // get last connected session, or create new one
  string session_id = UserTracker->query_last_session(user_id);
  if (!session_id || !is_active(session_id)) {
    session_id = SessionTracker->new_session(user_id);
    if (!session_id) {
      logger->warn("failed to start session: %O", user_id);
      return 0;
    }
  }

  // get the session avatar, or create a new one
  object avatar = SessionTracker->query_avatar(session_id);
  if (!avatar) {
    avatar = clone_object(PlatformAvatar);
    if (!avatar) {
      logger->warn("failed to clone platform avatar: %O %O", login, user_id);
      return 0;
    }
    SessionTracker->set_avatar(session_id, avatar);
  }

  mixed *args, ex;
  if (ex = catch(args = avatar->try_descend(session_id))) {
    logger->warn("caught exception in try_descend: %O", ex);
    return 0;
  } else {
    // (re)start session
    if (!SessionTracker->resume_session(session_id)) {
      logger->warn("failed to resume user session: %O %O", 
                   user_id, session_id);
      return 0; 
    }
    // switch connection to session
    if (!connect_session(login, session_id)) {
      logger->warn("failed to connect user session: %O %O", 
                   login, session_id);
      return 0;
    }
    apply(#'call_other, avatar, "on_descend", session_id, args);
    return session_id;
  } 
  return 0;
}
