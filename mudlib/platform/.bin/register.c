/**
 * User registration controller. Also will log user into game upon success.
 *
 * @alias RegisterController
 */
#include <topic.h>
#include <user.h>
#include <command.h>

inherit CommandController;

inherit CommandLib;
inherit MessageLib;
inherit ValidationLib;
inherit UserLib;

/**
 * Execute the register command. First creates the user, then starts a new
 * session and logs the user in.
 * 
 * @param  model         the command model
 * @param  verb          the command verb
 * @return an output model containing input username, new user id, and new 
 *         session id
 */
mapping execute(mapping model, string verb) {
  mapping result = ([
    "username" : model["username"],
  ]);

  // create new user
  advance_step(result);
  string user_id = create_user(model["username"], model["password"]);
  if (!user_id) {
    string msg = sprintf("User creation failed: username: %O\n", 
                         model["username"]); 
    stdout_msg(msg, ([ ]), THISP, TOPIC_LOGIN);
    return fail_result(result);
  }
  result["user_id"] = user_id;

  // attach to session
  advance_step(result);
  string session_id = attach_session(THISP, user_id);
  if (!session_id) {
    string msg = sprintf("Attaching user session failed: username: %O\n", 
                         model["username"]); 
    stdout_msg(msg, ([ ]), THISP, TOPIC_LOGIN);
    return fail_result(result);
  }
  result["session_id"] = session_id;

  return pass_result(result);
}

/**
 * Validate the password field matches the confirm password field.
 * 
 * @param  model         the command model
 * @return 1 if passwords match, otherwise 0
 */
int validate_passwords_match(mapping model) {
  return model["password"] == model["confirmPassword"];
}
