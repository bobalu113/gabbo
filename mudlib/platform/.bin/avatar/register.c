/**
 * User registration controller. Also will log user into game upon success.
 *
 * @alias RegistrationController
 */
#include <topic.h>
#include <user.h>

inherit CommandController;

private variables private functions inherit MessageLib;
private variables public functions inherit ValidationLib;

int execute(mapping model, string verb) {
  system_msg(THISP, "Registered!\n", ([ ]), TOPIC_LOGIN);
  return 1;
}

int validate_passwords_match(mapping model) {
  return model["password"] == model["confirmPassword"];
}


/**
 * Create a new user.
 * @param user     the username of the new user
 * @param password the user's password
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
 */

/**
 * Save user data to password file.
 *
 * @param  user      the username of the file
 * @param  data      the password data
 * @param  overwrite 1 to force overwrite of existing file, 0 otherwise
 * @return           1 for success, 0 for failure
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
 */
