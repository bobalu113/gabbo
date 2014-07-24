/**
 * A base object for user commands.
 * 
 * @author devo@eotl
 * @alias CommandCode
 */

inherit FileLib;

#include <sys/commands.h>

/**
 * Return a default list of actions to be associated with this command. The
 * result should be a two-dimensional array, with each inner array comprised
 * of two elements: a verb and an action flag (see add_action(E)).
 * 
 * @return an array of default actions
 */
mixed *query_actions() {
  return ({ ({ basename(load_name(THISO)), AA_VERB }) });
}

/**
 * Execute the command. Overload this function with code to be invoked each
 * time the command is run. Command-line arguments will be passed as the
 * string arg.
 * 
 * @param  arg command-line arguments
 * @return     1 for success, 0 for failure
 */
int do_command(string arg) {
  // FUTURE better fail message and some logging
  notify_fail("Not implemented.");
  return 0;
}

/**
 * Returns true to designate that this object is a command object.
 * 
 * @return 1 
 */
nomask int is_command() {
  return 1;
}