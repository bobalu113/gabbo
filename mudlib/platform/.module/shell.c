/**
 * The ShellMixin provides the features of a Unix-like shell.
 * @alias ShellMixin
 */
#include <capabilities.h>
#include <command_giver.h>

// TODO environment variables
// TODO prompt
// TODO aliases
// TODO subs

inherit FileLib;

default private variables;

private mapping CAPABILITIES_VAR = ([ CAP_SHELL ]);
private string CMD_IMPORTS_VAR = PlatformBinDir "/shell/shell.cmds";

string cwd, homedir, *dirstack, context;

default private functions;

/**
 * Setup the ShellMixin.
 */
protected void setup() {
  dirstack = ({ });
  context = "";
}

/**
 * Get the player's current working directory.
 *
 * @return the player's current working directory
 */
public string query_cwd() {
  return cwd;
}

/**
 * Set the current working directory for this player.
 *
 * @param  dir the directory to set
 * @return     0 for failure, 1 for success
 */
public int set_cwd(string dir) {
  // TODO add security
  if (!is_directory(dir)) {
    return 0;
  }
  cwd = dir;
  return 1;
}

/**
 * Return the directory stack as an array, starting with the most recently
 * added directory.
 *
 * @return the directory stack
 */
public string *query_dirs() {
  return dirstack;
}

/**
 * Push a directory onto the directory stack.
 *
 * @param dir the directory to add
 */
public void push_dir(string dir) {
  dirstack = ({ dir }) + dirstack;
  return;
}

/**
 * Remove the last pushed directory from the directory stack and return it.
 *
 * @return the most recently pushed directory
 */
public string pop_dir() {
  if (!sizeof(dirstack)) {
    return 0;
  }
  string dir = dirstack[0];
  dirstack = dirstack[1..];
  return dir;
}

/*
  Clear the directory stack.
 */
public void clear_dirs() {
  dirstack = ({ });
  return;
}

/**
 * Return the player's home directory.
 *
 * @return the player's home directory.
 */
public string query_homedir() {
  return homedir;
}

/**
 * Set the home directory for this player.
 *
 * @param  dir the directory to set
 * @return     0 for failure, 1 for success
 */
public int set_homedir(string dir) {
  // TODO add security
  if (!is_directory(dir)) {
    return 0;
  }
  homedir = dir;
  return 1;
}

/**
 * Get the user's current context for object expansion.
 *
 * @return the current context
 */
public string query_context() {
  return context;
}

/**
  * Set the user's current context for object expansion.
 * @param  c context to set
 * @return   1 for success, 0 for failure
 */
public int set_context(string c) {
  LoggerFactory->get_logger(THISO)->trace("set_context(%O)", c);
  if (!stringp(c)) {
    return 0;
  }
  context = c;
  return 1;
}
