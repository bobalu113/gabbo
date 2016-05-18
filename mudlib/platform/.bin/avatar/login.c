#include <topic.h>

private variables private functions inherit MessageLib;

inherit CommandController;

int execute(mapping model, string verb) {
  system_msg(THISO, "Logged in!", ([ ]), TOPIC_LOGIN);
  return 1;
}

/**
 * Spawn a new avatar object, move to starting location, and transfer
 * interactivity from login object to new avatar.
 *
 * @param user the username of the new avatar
 */
/*
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
*/