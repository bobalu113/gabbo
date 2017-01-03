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
inherit FormatStringsLib;

struct UserInfo {
  string id;
  string username;
  string last_session;
};

string user_dir(string username);
int user_exists(string username);
string passwd_file(string username);
string hash_passwd(string password);
string create_user(string username, string password);
int install_skeleton(string user_dir);
int apply_template(string template, string username);
string attach_session(object login, string user_id);

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
  if (!install_skeleton(user_dir)) {
    logger->warn("unable to install skeleton user dir: %O", user_dir);
  }

  string user_id = UserTracker->new_user(username);

  if (!save_password(user_id, password)) {
    logger->warn("unable to save password for user: %O %O", user_id, username);
  }
  
  return user_id;
}

int install_skeleton(string user_dir) {
  object logger = LoggerFactory->get_logger(THISO);
  if (file_exists(user_dir)) {
    return 0;
  }
  copy_tree(SkelDir, user_dir);
  if (!apply_template(user_dir + DOMAIN_TEMPLATE, username)) {
    logger->warn("unable to apply domain template: %O", user_dir);
  }
  if (!apply_template(user_dir + ZONE_TEMPLATE, username)) {
    logger->warn("unable to apply zone template: %O", user_dir);
  }
  return 1;
}

int apply_template(string template_path, string username) {
  if ((strlen(template_path) < 10) 
      || (template_path[<9..<1] != TEMPLATE_SUFFIX)) {
    return 0;
  }
  mapping infomap = ([ 
    'f' : ({ 0,
             "%s",
             ({ "gabbo-basic" }) // TODO support user flavors
          }),
    'u' : ({ 0,
             "%s",
             ({ username }) 
          })
  ]);
  string template = read_file(template_path);
  if (!template) {
    return 0;
  }
  string body = funcall(parse_format(template, infomap, 0));
  if (!write_file(template_path[0..<10], body)) {
    return 0;
  }
  if (!rm(template_path)) {
    return 0;
  }
  return 1;
}

int save_password(string user_id, string password) {
  object logger = LoggerFactory->get_logger(THISO);
  string username = UserTracker->query_username(user_id);
  string passwd_file = passwd_file(username);
  mapping passwd = read_value(passwd_file);
  if (!mappingp(passwd)) {
    passwd = ([ ]);
    if (file_exists(passwd_file)) {
      logger->warn("Overwriting corrupt password file %O", passwd_file);
    }
  }
  string hash = hash_passwd(password);
  passwd[user_id] = ([ PASSWD_PASSWORD: hash ]);
  write_value(passwd_file, passwd);
  return 1;
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
  if (ex = catch(args = avatar->try_descend(session_id); publish)) {
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
