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

#define DEFAULT_SCREEN_WIDTH   80
#define DEFAULT_SCREEN_LENGTH  25
#define AvatarBinDir           PlatformBinDir "/avatar"

mapping CAPABILITIES_VAR = ([ CAP_AVATAR ]);
string CMD_IMPORTS_VAR = AvatarBinDir "/avatar.xml";

/**
 * Returns the user's screen width. Currently hard-coded to 80 characters.
 * @return the user's screen width
 */
public int query_screen_width() {
  struct ConnectionInfo info = 
    ConnectionTracker->query_connection_info(
      ConnectionTracker->query_connection_id(THISO)
    );
  int result = info->terminal_width;
  if (result <= 0) {
    result = DEFAULT_SCREEN_WIDTH;
  }
  return result;
}

/**
 * Returns the user's screen length. Currently hard-coded to 25 lines.
 * @return the user's screen length
 */
public int query_screen_length() {
  struct ConnectionInfo info = 
    ConnectionTracker->query_connection_info(
      ConnectionTracker->query_connection_id(THISO)
    );
  int result = info->terminal_width;
  if (result <= 0) {
    result = DEFAULT_SCREEN_LENGTH;
  }
  return result;
}

/**
 * Return the user's terminal type, as detected by telnet negotation.
 * 
 * @return a string designating the terminal type
 */
public string query_terminal_type() {
  struct ConnectionInfo info = 
    ConnectionTracker->query_connection_info(
      ConnectionTracker->query_connection_id(THISO)
    );
  if (info) {
    return info->terminal;
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
  if (interactive(THISO)) {
    ConnectionTracker->telnet_get_terminal(THISO);
    ConnectionTracker->telnet_get_NAWS(THISO);
    ConnectionTracker->telnet_get_ttyloc(THISO);
  }
}
