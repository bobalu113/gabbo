/**
 * A module for interactive objects.
 *
 * @author devo@eotl
 * @alias AvatarMixin
 */

#include <capabilities.h>
#include <command_giver.h>

private variables private functions inherit ConnectionLib;
private variables private functions inherit ObjectLib;

#define AvatarBinDir        PlatformBinDir "/avatar"

mapping CAPABILITIES_VAR = ([ CAP_AVATAR ]);
mapping CMD_IMPORTS_VAR = ([ 
  AvatarBinDir "/login", 
  AvatarBinDir "/register", 
  AvatarBinDir "/guest", 
  AvatarBinDir "/quit" 
]);

/**
 * Returns the user's screen width. Currently hard-coded to 80 characters.
 * @return the user's screen width
 */
public int query_screen_width() {
  return 80;
}

/**
 * Returns the user's screen length. Currently hard-coded to 25 lines.
 * @return the user's screen length
 */
public int query_screen_length() {
  return 25;
}

/**
 * Return the user's terminal type, as detected by telnet negotation.
 * 
 * @return a string designating the terminal type
 */
public string query_terminal_type() {
  struct ConnectionConfig config = 
    ConnectionTracker->query_config(
      ConnectionTracker->query_connection_id(THISO)
    );
  if (config) {
    return config->terminal;
  }
  return 0;
}

/**
 * Restore the default prompt.
 */
public void restore_prompt() {
  if (is_capable(THISO, CAP_SHELL)) {
    set_prompt(lambda(0,
      ({ #'+,
        ({ #'call_other, THISO, "query_context" }),
        "> "
      })
    ), THISO);
  } else {
    set_prompt("> ");
  }
}

/**
 * Implementation of the modify_command hook. Every command executed by the
 * player will have a chance to be modified before being parsed by this
 * function.
 *
 * @param  cmd the command string being executed
 * @return     the new command string to execute
 */
mixed modify_command(string cmd) {
  // TODO add support for default exit verb setting
  if (ENV(THISO) && ENV(THISO)->query_exit(cmd)) {
    return "walk " + cmd;
  }
  return 0;
}

/**
 * Returns true to designate that this object represents an avatar.
 *
 * @return 1
 */
nomask int is_avatar() {
  return 1;
}

/**
 * Initialize AvatarMixin. If this function is overloaded, be advised
 * that the mixin's private variables are initialized in the parent
 * implementation.
 */
void setup_avatar() {
}
