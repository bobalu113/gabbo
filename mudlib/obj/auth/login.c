#include <ansi.h>
#include <user.h>
#include <sys/input_to.h>

inherit UserLib;

#define WELCOME_FILE $EtcDir "/issue"
#define TIMEOUT 180
#define MAX_TRIES 3

int logon();
static void select_password(string password, int tries, string user);
static void confirm_password(string cpassword, int tries, string user, 
                             string password);
static void start_timeout();
static void timeout();
static void abort();
static void spawn_avatar(string user);
static void new_user(string user, string password);
static int save_passwd(string user, mapping data, int overwrite);

int logon() {
  if (caller_stack_depth() > 0) {
    return 0;
  }
  printf(CLEAR_SCREEN);
  printf(read_file(WELCOME_FILE));
  input_to("select_user", INPUT_PROMPT|INPUT_IGNORE_BANG, "Enter username: ", 
    0);
  start_timeout();
  return 1;
}

static void select_user(string user, int tries) {
  if (!strlen(user)) {
    if (tries >= MAX_TRIES) {
        abort();
    } else {
      input_to("select_user", INPUT_PROMPT|INPUT_IGNORE_BANG, 
        "Enter username: ", tries + 1);
      start_timeout();
    }
  } else {
    string prompt = "Enter password: ";
    if (!user_exists(user)) {
      prompt = "New user. Choose a password: ";
    } 
    input_to("select_password", INPUT_NOECHO|INPUT_PROMPT|INPUT_IGNORE_BANG,
      prompt, 0, user);
    start_timeout();
  }
}

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
      start_timeout();
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
          start_timeout();
        }
      }
    } else {
      input_to("confirm_password", INPUT_NOECHO|INPUT_PROMPT
        |INPUT_IGNORE_BANG, "Confirm password: ", tries, user, password);
      start_timeout();
    }
  }  
}

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
        start_timeout();
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
      start_timeout();
    }
  }
}

static void start_timeout() {
  if (find_call_out("timeout") != -1) {
    remove_call_out("timeout");
    call_out("timeout", TIMEOUT);
  }
}

static void timeout() {
  printf("Timeout exceeded, disconnecting...");
  abort();
}

static void abort() {
  remove_call_out("timeout");
  destruct(THISO);
}

static void spawn_avatar(string user) {
  string err;

  mapping data = restore_value(read_file(PASSWD_FILE(user)));
  data["connect_time"] = time();
  data["disconnect_time"] = -1;
  if (!save_passwd(user, data, 1)) {
    // TODO log info
  }

  // FUTURE implement 2-tier character selection

  object avatar;
  string avatar_name;
  err = catch (
    (avatar = clone_object(AVATAR)),
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
      // TODO log warning
      move_object(avatar, location);
      publish
    );
    if (err) {
      printf("Caught error moving %O to starting location %O for user "
        "%s: %s\n", avatar, location, user, err); 
    }
  }

  if (exec(avatar, THISO)) {
    set_this_player(avatar);
    avatar->enter_game();
    destruct(THISO);
  } else {
    // TODO log error
    printf("Unable to exec user %s into avatar %O\n", user, avatar);
    destruct(avatar);
    destruct(THISO);
  }
}

static void new_user(string user, string password) {
  // TODO encrypt passwords
  mapping data = ([ "username" : user,
                    "password" : password,
                    "location" : COMMON_ROOM,
                    "connect_time" : -1,
                    "disconnect_time" : -1 ]);
  if (!save_passwd(user, data, 0)) {
    destruct(THISO);
  } else {
    if (!file_exists(HomeDir "/" + user)) {
      mkdir(HomeDir "/" + user);
    }
    spawn_avatar(user);
  }
}

static int save_passwd(string user, mapping data, int overwrite) {
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
