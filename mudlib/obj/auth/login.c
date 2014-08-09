/**
 * A login object to handle incoming connections.
 * 
 * @author devo@eotl
 * @alias LoginObject
 */
#include <ansi.h>
#include <user.h>
#include <sys/input_to.h>

#define WELCOME_FILE EtcDir "/issue"
#define TIMEOUT 10
#define MAX_TRIES 3

private functions private variables inherit UserLib;
private functions private variables inherit FileLib;

int logon();
static void select_user(string user, int tries);
static void select_password(string password, int tries, string user);
static void confirm_password(string cpassword, int tries, string user, 
                              string password);
public void restart_timeout();
static void timeout();
static void abort();
protected void spawn_avatar(string user);
protected void new_user(string user, string password);
protected int save_passwd(string user, mapping data, int overwrite);

/**
 * Invoked by the master object when a new connection is established.
 * 
 * @return 1 for success, 0 for failure
 */
int logon() {
  if (caller_stack_depth() > 0) {
    return 0;
  }
  printf(CLEAR_SCREEN);
  printf(read_file(WELCOME_FILE));
  input_to("select_user", INPUT_PROMPT|INPUT_IGNORE_BANG, "Enter username: ", 
    0);
  restart_timeout();
  return 1;
}

/**
 * Read input from user for username.
 * 
 * @param user  the inputted username 
 * @param tries the number of tries used
 */
static void select_user(string user, int tries) {
  if (!strlen(user)) {
    if (tries >= MAX_TRIES) {
        abort();
    } else {
      input_to("select_user", INPUT_PROMPT|INPUT_IGNORE_BANG, 
        "Enter username: ", tries + 1);
      restart_timeout();
    }
  } else {
    string prompt = "Enter password: ";
    if (!user_exists(user)) {
      prompt = "New user. Choose a password: ";
    } 
    input_to("select_password", INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
      prompt, 0, user);
    restart_timeout();
  }
}

/**
 * Read input from user for password.
 * 
 * @param password  the inputted password 
 * @param tries     the number of tries used
 * @param user      the selected username
 */
static void select_password(string password, int tries, string user) {
  printf("\n");
  if (!strlen(password)) {
    if (tries >= MAX_TRIES) {
        abort();
    } else {
      string prompt = "Enter password: ";
      if (!user_exists(user)) {
        prompt = "Choose a password: ";
      } 
      input_to("select_password", INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
        prompt, tries + 1, user);
      restart_timeout();
    }
  } else {
    if (user_exists(user)) {
      if (valid_password(user, password)) {
        remove_call_out("timeout");
        spawn_avatar(user);
      } else {
        if (tries >= MAX_TRIES) {
          printf("Maximum attempts exceeded, disconnecting.\n");
          abort();
        } else {
          printf("Invalid password, try again. ");
          input_to("select_password", INPUT_NOECHO|INPUT_PROMPT
            |INPUT_IGNORE_BANG, "Enter password: ", tries + 1, user);
          restart_timeout();
        }
      }
    } else {
      input_to("confirm_password", INPUT_NOECHO|INPUT_PROMPT
        |INPUT_IGNORE_BANG, "Confirm password: ", tries, user, password);
      restart_timeout();
    }
  }  
}

/**
 * Read the password again from new users for confirmation.
 * 
 * @param cpassword the confirmed password
 * @param tries     the number of tries used
 * @param user      the selected username
 * @param password  the selected password
 */
static void confirm_password(string cpassword, int tries, string user, 
                              string password) {
  printf("\n");
  if (cpassword == password) {
    remove_call_out("timeout");
    if (user_exists(user)) {
      if (valid_password(user, password)) {
        spawn_avatar(user);
      } else {
        printf("Username %s already taken, try again. ");
        input_to("select_user", INPUT_PROMPT|INPUT_IGNORE_BANG, 
          "Enter username: ", 0);
        restart_timeout();
      }
    } else {
      new_user(user, password);
    }
  } else {
    if (tries >= MAX_TRIES) {
      printf("Maximum attempts exceeded, disconnecting.\n");
      abort();
    } else {
      printf("Passwords do not match, try again. ");
      input_to("select_password", INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG, 
        "Choose a password: ", tries + 1, user);
      restart_timeout();
    }
  }
}

/**
 * Restart idle timeout after user input.
 */
public void restart_timeout() {
  if (find_call_out("timeout") != -1) {
    remove_call_out("timeout");
  }
  call_out("timeout", TIMEOUT);
}

/**
 * Exit login prompt because of idle timeout.
 */
static void timeout() {
  printf("Timeout exceeded, disconnecting...\n");
  abort();
}

/**
 * Abort login.
 */
static void abort() {
  remove_call_out("timeout");
  destruct(THISO);
}

/**
 * Spawn a new avatar object, move to starting location, and transfer 
 * interactivity from login object to new avatar.
 * 
 * @param user the username of the new avatar
 */
protected void spawn_avatar(string user) {
  object logger = LoggerFactory->getLogger(THISO);
  string err;

  mapping data = restore_value(read_file(PASSWD_FILE(user)));
  data["connect_time"] = time();
  data["disconnect_time"] = -1;
  if (!save_passwd(user, data, 1)) {
    // TODO give friendly warning
    logger->info("Unable to save password data for user %s", user);
  }

  // FUTURE implement 2-tier character selection

  object avatar;
  string avatar_name;
  err = catch (
    (avatar = clone_object(Avatar)),
    (avatar_name = object_name(avatar)),
    (avatar->setup(user) && (
      destruct(avatar),
      throw(sprintf("%s reported setup failure for user %s\n", 
        avatar_name, user))
    ));
    publish
  );
  if (err) {
    printf("Caught error setting up avatar for user %s: %s\n", user, err); 
    destruct(THISO);
    return;
  }

  object location;
  err = catch (location = load_object(data["location"]); publish);
  if (err) {
    printf("Caught error loading starting location for user %s: %s\n", user, 
      err); 
  } else {
    err = catch (
      move_object(avatar, location);
      publish
    );
    if (err) {
      // TODO  give friendly error
      logger->info("Caught error moving %O to starting location %O for user "
        "%s: %s\n", avatar, location, user, err); 
      printf("Caught error moving %O to starting location %O for user "
        "%s: %s\n", avatar, location, user, err); 
    }
  }

  if (exec(avatar, THISO)) {
    set_this_player(avatar);
    avatar->enter_game();
    destruct(THISO);
  } else {
    logger->error("Unable to exec user %s into avatar %O\n", user, avatar);
    printf("Unable to exec user %s into avatar %O\n", user, avatar);
    destruct(avatar);
    destruct(THISO);
  }
}

/**
 * Create a new user.
 * @param user     the username of the new user
 * @param password the user's password
 */
protected void new_user(string user, string password) {
  // TODO encrypt passwords
  mapping data = ([ "username" : user,
                    "password" : password,
                    "location" : CommonRoom,
                    "connect_time" : -1,
                    "disconnect_time" : -1 ]);
  if (!save_passwd(user, data, 0)) {
    destruct(THISO);
  } else {
    string homedir = HomeDir + "/" + user;
    if (!file_exists(homedir)) {
      mkdir(homedir);
    }
    spawn_avatar(user);
  }
}

/**
 * Save user data to password file.
 * 
 * @param  user      the username of the file
 * @param  data      the password data
 * @param  overwrite 1 to force overwrite of existing file, 0 otherwise
 * @return           1 for success, 0 for failure
 */
protected int save_passwd(string user, mapping data, int overwrite) {
  string passwd_file = PASSWD_FILE(user);
  if (file_exists(passwd_file)) {
    if (overwrite) {
      if (!rm(passwd_file)) {
        printf("Unable to remove old password file for user %s. Contact an "
          "administrator.\n", user);
        return 0;
      }
    } else {
      printf("Password file for user %s already exists. Contact an "
        "administrator.\n", user);
      return 0;
    }
  }
  if (!write_file(passwd_file, save_value(data))) {
    printf("Unable to write password file for user %s. Contact an "
      "administrator.\n", user);
    return 0;
  }
  return 1;
}
