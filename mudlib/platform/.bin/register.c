/**
 * User registration controller. Also will log user into game upon success.
 *
 * @alias RegistrationController
 */
#include <topic.h>
#include <user.h>

inherit CommandController;

inherit MessageLib;
inherit ValidationLib;
inherit UserLib;

int execute(mapping model, string verb) {
  // create new user
  string user_id = create_user(model["username"], model["password"]);
  if (!user_id) {
    string msg = sprintf("User creation failed: username: %O", 
                         model["username"]); 
    system_msg(THISP, msg, ([ ]), TOPIC_LOGIN);
    return 1;
  }

  // attach to session
  string session_id = attach_session(THISP, user_id);
  if (!session_id) {
    string msg = sprintf("Attaching user session failed: username: %O", 
                         model["username"]); 
    system_msg(THISP, msg, ([ ]), TOPIC_LOGIN);
    return 1;
  }

  return 1;
}

int validate_passwords_match(mapping model) {
  return model["password"] == model["confirmPassword"];
}
