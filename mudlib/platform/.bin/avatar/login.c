inherit CommandCode;

int do_command(string arg) {
  if (stringp(arg) && strlen(arg) > 0) {
    notify_fail(sprintf("%s: Too many arguments.\n", query_verb()));
    return 0;
  }
  printf("%s\n", THISP->query_cwd());
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
 * Spawn a new avatar object, move to starting location, and transfer
 * interactivity from login object to new avatar.
 *
 * @param user the username of the new avatar
 */
protected void enter_game(string user) {
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
