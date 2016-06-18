/**
 * Utility library for tracking sessions.
 *
 * @author devo@eotl
 * @alias SessionLib
 */

struct SessionInfo {
  string id;
  string user;
  int login_time;
  int logout_time;
  int invisible;
  string *connections;
};