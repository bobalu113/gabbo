/**
 * A module for objects to execute user commands.
 *
 * @author devo@eotl
 * @alias CommandGiverMixin
 */

#include <sys/functionlist.h>
#include <capabilities.h>
#include <command_giver.h>
#include <command.h>

inherit ObjectLib;
inherit FileLib;
inherit ArrayLib;
inherit CommandSpecLib;

mapping CAPABILITIES_VAR = ([ CAP_COMMAND_GIVER ]);

default private variables;

nosave mixed *commands;

default public functions;

private mixed *load_command_spec(string specfile);
object load_controller(string controller);

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
      result += ({ ({ val, load_command_spec(val) }) });
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
void setup() {
  commands = load_commands();
  object logger = LoggerFactory->get_logger(THISO);
  //logger->debug("commands: %O\n", commands);
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
  mapping fragments = ([ ]); // len to fragment 
  mapping matched = ([ ]);  // verb to arg
  int arglen = strlen(arg);
  int result = 0;
  foreach (mixed *cmd : commands) {
    string specfile = cmd[0];
    foreach (mixed *command : cmd[1]) {
      foreach (string verb : command[COMMAND_VERBS]) {
        if (member(matched, verb)) {
          if (matched[verb]) {
            object controller = load_controller(expand_path(
                                  command[COMMAND_CONTROLLER], specfile));
            if (controller) {
              result = controller->do_command(command, verb, matched[verb]);
            }
            // TODO notify broken controller via result
            if (result) {
              break;
            }
          } else {
            continue;
          }
        }
        int len = strlen(verb);
        if (len > arglen) {
          continue;
        }
        if (!member(fragments, len)) {
          fragments[len] = arg[0..(len - 1)];
        }
        if (fragments[len] == verb) {
          if (len == arglen) {
            matched[verb] = "";
          } else if (arg[len] == ' ') {
            // trim leading spaces
            int i = len;
            while (arg[i] == ' ') { 
              i++;
            }
            // set the match
            if (i < arglen) {
              matched[verb] = arg[i..];
            } else {
              matched[verb] = "";
            }
          } else {
            matched[verb] = 0;
          }
        } else {
          matched[verb] = 0;
        }

        if (matched[verb]) {
          object controller = load_controller(expand_path(
                                command[COMMAND_CONTROLLER], specfile));
          if (controller) {
            result = controller->do_command(command, verb, matched[verb]);
          }
          if (result) {
            break;
          }
        }
      }
      if (result) {
        break;
      }
    }
    if (result) {
      break;
    }    
  }
  
  return result;
}

int run_script(string script_file) {
  string script = read_file(script_file);
  if (!script) {
    return 0;
  }
  string *lines = explode(script, "\n");
  foreach (string line : lines) {
    command(line);
  }
  return 1;
}

object load_controller(string controller) {
  object result;
  object logger = LoggerFactory->get_logger(THISO);
  string err = catch(result = load_object(controller); publish);
  if (err) {
    logger->info("error loading controller %s: %s", controller, err);
  }
  return result;
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
