/**
 * A module for objects to execute user commands.
 *
 * @author devo@eotl
 * @alias CommandGiverMixin
 */

#include <sys/functionlist.h>
#include <capabilities.h>
#include <command_giver.h>

private variables private functions inherit ObjectLib;
private variables private functions inherit ArrayLib;
private variables private functions inherit CommandSpecLib;

mapping CAPABILITIES_VAR = ([ CAP_COMMAND_GIVER ]);

default private variables;

nosave mixed *commands;

default public functions;

private mixed *load_command_spec(string specfile);

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

mixed *load_commands() {
  mixed *result = ({ });
  mixed *vars = variable_list(THISO, RETURN_FUNCTION_NAME
                                     | RETURN_VARIABLE_VALUE);
  int i = 0;
  while ((i = member(vars,  CMD_IMPORTS_VAR_STR, i)) != -1) {
    mixed val = vars[++i];
    if (stringp(val)) {
      result += load_command_spec(val);
    }
    i++;
  }
  return result;
}

private mixed *load_command_spec(string specfile) {
  mixed *result = ({ });
  mixed *xml = xml_parse(read_file(specfile));
  object logger = LoggerFactory->get_logger(THISO);

  return parse_commands_xml(specfile, xml);
}

/**
 * Initialize CommandGiverMixin. If this function is overloaded, be advised
 * that the mixin's private variables are initialized in the parent
 * implementation.
 */
void setup_command_giver() {
  commands = load_commands();
  object logger = LoggerFactory->get_logger(THISO);
  logger->debug("commands: %O\n", commands);
}

/**
 * Main command router. Any added commands will pass through this action
 * function before being routed to the proper command object.
 *
 * @param  arg the command-line argument
 * @return     the result of the command execution; 1 for success, 0 for
 *             failure.
 */
int do_command(string arg) {

  int ret = random(2);
  object logger = LoggerFactory->get_logger(THISO);
  logger->debug("%O,%O,%O,%O", ret, query_notify_fail(), efun::query_command(), arg);
  return ret;
/*  
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
*/
}
