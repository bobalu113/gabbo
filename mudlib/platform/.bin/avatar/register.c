/**
 * User registration controller. Also will log user into game upon success.
 *
 * @alias RegistrationController
 */
#include <topic.h>
#include <user.h>

inherit CommandController;

inherit FileLib;
inherit MessageLib;
inherit ValidationLib;

int execute(mapping model, string verb) {
  string user_id = create_user(model["username"], model["password"]);
  if (!user_id) {
    string msg = sprintf("User creation failed: username: %O", 
                         model["username"]); 
    system_msg(THISP, msg, ([ ]), TOPIC_LOGIN);
    return 1;
  }

  string session_id = start_session(user_id);
  if (!session_id) {
    string msg = sprintf("Starting session failed: username: %O", 
                         model["username"]); 
    system_msg(THISP, msg, ([ ]), TOPIC_LOGIN);
    return 1;
  }

  return 1;
}

int validate_passwords_match(mapping model) {
  return model["password"] == model["confirmPassword"];
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
  string user_id = UserTracker->new_user(username, password);

  // save password
  mapping passwd = read_value(passwd_file(username));
  if (!mappingp(passwd)) {
    passwd = ([ ]);
  }
  passwd[user_id] = ([ PASSWD_PASSWORD: hash ]);
  write_value(passwd_file(username), passwd);
  
  return user_id;
}

string start_session(string user_id) {
  object logger = LoggerFactory->get_logger(THISO);
  object login = THISP;
  object avatar = clone_object(PlatformAvatar);
  if (!avatar) {
    logger->warn("failed to clone platform avatar: %O %O", THISP, user_id);
    return 0;
  }

  mixed *args, ex;
  if (ex = catch(args = avatar->try_descend(user_id, login))) {
    logger->warn("caught exception in try_descend: %O", ex);
    return 0;
  } else {
    if (!switch_connection(login, avatar)) {
      logger->warn("failed to switch connection from login to avatar: %O %O", 
                   login, avatar);
      return 0;
    }
    string session_id = SessionTracker->new_session(user_id);
    if (!session_id) {
      logger->warn("failed to start session: %O", user_id);
      return 0;
    }
    if (!UserTracker->session_start(user_id, session_id)) {
      logger->warn("failed to attach user session: %O %O", 
                   user_id, session_id);
      return 0;
    }
    apply(#'call_other, avatar, "on_descend", session_id, login, args);
    return session_id;
  } 
  return 0;
}
