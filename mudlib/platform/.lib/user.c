/**
 * Utility library for dealing with users.
 * 
 * @author devo@eotl
 * @alias UserLib
 */
#include <user.h>

inherit FileLib;

struct UserInfo {
  string id;
  string username;
  string last_session;
}

/**
 * Get the user directory for a given username.
 * 
 * @param  username the username of directory owner
 * @return          the user dir
 */
string user_dir(string username) {
  return UserDir "/" + username;
}

/**
 * Test whether a user with a given name exists.
 * 
 * @param  username the username to test
 * @return          1 if the user exists, otherwise 0
 */
int user_exists(string username) {
  return file_exists(user_dir(username));
}

/**
 * Test whether a password is valid for the specified user.
 * 
 * @param  username the username to test
 * @param  password the password
 * @return          1 if the password matches the password on file, otherwise 
 *                  0
 */
int valid_password(string username, string password) {
  mapping data = restore_value(read_file(PASSWD_FILE(username)));
  return password == data["password"];
}
