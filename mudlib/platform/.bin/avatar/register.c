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
        enter_game(user);
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
