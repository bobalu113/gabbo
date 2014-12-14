/**
 * The ShellMixin provides the features of a Unix-like shell.
 * @alias ShellMixin
 */

// TODO environment variables
// TODO prompt
// TODO aliases
// TODO subs

private variables private functions inherit FileLib;

default private variables;

string cwd;

default private functions;

/**
 * Return the players current working directory.
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
 * Initialize ShellMixin. 
 */
protected void setup_shell() {
}
