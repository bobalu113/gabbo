/**
 * The ShellMixin provides the features of a Unix-like shell.
 * @alias ShellMixin
 */

#include <capabilities.h>

// TODO environment variables
// TODO prompt
// TODO aliases
// TODO subs

private variables private functions inherit FileLib;

default private variables;

string cwd, homedir, *dirstack;

default private functions;

/**
 * Return the player's current working directory.
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
 * Initialize ShellMixin.
 */
protected void setup_shell() {
  dirstack = ({ });
  return;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 *
 * @return a zero-width mapping of capabilities
 */
public mapping query_capabilities() {
  return ([ CAP_SHELL ]);
}
