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

string cwd, homedir;

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
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 * @return a zero-width mapping of capabilities
 */
mapping query_capabilities() {
  return ([ CAP_SHELL ]);
}
