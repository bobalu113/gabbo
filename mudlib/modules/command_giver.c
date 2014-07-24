/**
 * A module for objects to execute user commands.
 * 
 * @author devo@eotl
 * @alias CommandGiverMixin
 */

default private variables;

// ([ verb : command_file; flag ])
mapping verbs;

// to keep track if a command has been reloaded since init
// ([ command_object ])
nosave mapping commands;

default public functions;

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
 * Add a new command to the user's command map.
 * 
 * @param  command the program name of the command object
 * @param  verb    the verb which will execute the command
 * @param  flag    optional action flag (see add_action(E))
 * @return         0 for failure, 1 for success
 */
protected varargs int add_command(string command, string verb, int flag) {
  object cmd_ob;
  string err = catch (cmd_ob = load_object(command));
  // TODO consolidate command loading logic (see above)
  if (err) {
    // TODO log error
    printf("Caught error loading command %s: %s\n", command, err);
    return 0;
  } 

  commands += ([ cmd_ob ]);
  verbs += ([ verb : command; flag ]);
  return 1;
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
    // TODO log warning
    return 0;
  }
  string command = verbs[verb];
  object cmd_ob = FINDO(command);
  if (!cmd_ob || !member(commands, cmd_ob)) {
    // FUTURE prompt user whether or not to re-init
    string err = catch (cmd_ob = load_object(command));
    // TODO consolidate command loading logic (see above)
    if (err) {
      // TODO log error
      printf("Caught error loading command %s: %s\n", command, err);
      return 0;
    } else {
      commands += ([ cmd_ob ]);
    }
  }

  return cmd_ob->do_command(arg);
}

