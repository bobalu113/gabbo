/**
 * A module for objects to execute user commands.
 *
 * @author devo@eotl
 * @alias CommandGiverMixin
 */

#include <capabilities.h>
#include <command_giver.h>

default private variables;

/**
 * ([ verb : ({ ({ command_file, flag }), ... }) ])
 */
mapping verbs;   // TODO command_file needs to be an array

/**
 * To keep track if a command object has been reloaded since init.
 * <code>([ command_object ])</code>
 */
nosave mapping commands;

default public functions;

protected object load_command(string command);
protected varargs int add_command(string command, string verb, int flag);
static int do_command(string arg);

/**
 * Return the mapping of all configured command verbs. Keys are the verbs,
 * the first value is the program name of the command object, the second
 * value is an action flag (see add_action(E)).
 *
 * @return the mapping of verbs to commands
 */
mapping query_verbs() {
  return verbs;
}

/**
 * Return the command for a given verb.
 *
 * @param  verb the verb which is being executed
 * @return      the program name of the command object used for that verb
 */
string query_command(string verb) {
  return verbs[verb, 0];
}

/**
 * Return the action flags for a given verb. (see add_action(E)).
 *
 * @param  verb the verb which is being executed
 * @return      the action flag for the given verb
 */
int query_command_flag(string verb) {
  return verbs[verb, 1];
}

/**
 * Load a command object (and log errors).
 *
 * @param  command the command to load
 * @return         the loaded command object, or 0 if could not be loaded
 */
protected object load_command(string command) {
  object cmd_ob;
  string err = catch (cmd_ob = load_object(command); publish);
  if (err) {
    object logger = LoggerFactory->get_logger(THISO);
    logger->error("Caught error loading command object %s: %s", command, err);
    return 0;
  }
  return cmd_ob;
}

/**
 * Add a new command to the user's command map.
 *
 * @param  command the program name of the command object
 * @param  verb    the verb which will execute the command
 * @param  flag    optional action flag (see add_action(E))
 * @return         0 for failure, 1 for success
 */
protected varargs int add_command(string command, string verb, int flag) {
  object cmd_ob = load_command(command);
  if (cmd_ob) {
    commands += ([ cmd_ob ]);
    verbs += ([ verb : command; flag ]);
    return 1;
  }
  return 0;
}

/**
 * Test whether a given command object should be allowed this command giver's
 * read/write access.
 *
 * @param  cmd_ob the command object being executed
 * @param  read   non-zero if only read access is being granted
 * @return        non-zero if access should be granted
 */
int check_command_access(object cmd_ob, int read) {
  int config = GRANT_ACCESS_UNFORCED;
  if (THISP != THISO) {
    return 0;
  }
  switch (config) {
    case GRANT_ACCESS_ALL:
      return 1;
    case GRANT_ACCESS_UNFORCED:
      return (THISP == THISI);
    case GRANT_ACCESS_NONE:
    default:
      return 0;
  }
  return 0;
}

/**
 * Initialize CommandGiverMixin. If this function is overloaded, be advised
 * that the mixin's private variables are initialized in the parent
 * implementation.
 */
void setup_command_giver() {
  verbs = ([ ]);
  commands = ([ ]);
}

/**
 * Main command router. Any added commands will pass through this action
 * function before being routed to the proper command object.
 *
 * @param  arg the command-line argument
 * @return     the result of the command execution; 1 for success, 0 for
 *             failure.
 */
static int do_command(string arg) {
  string verb = query_verb(1);
  if (!member(verbs, verb)) {
    object logger = LoggerFactory->get_logger();
    logger->warn("Trying to route command for unknown verb: %s", verb);
    return 0;
  }
  string command = verbs[verb];
  object cmd_ob = FINDO(command);
  if (!cmd_ob || !member(commands, cmd_ob)) {
    // FUTURE prompt user whether or not to re-init
    cmd_ob = load_command(command);
    if (cmd_ob) {
      commands += ([ cmd_ob ]);
    }
  }

  return cmd_ob->do_command(arg);
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 *
 * @return a zero-width mapping of capabilities
 */
public mapping query_capabilities() {
  return ([ CAP_COMMAND_GIVER ]);
}
