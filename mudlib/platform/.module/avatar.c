/**
 * A module to support chains of avatars.
 *
 * @author devo@eotl
 * @alias AvatarMixin
 */
#include <capabilities.h>

inherit CommandGiverMixin;
inherit SensorMixin;
inherit ShellMixin;

inherit ArrayLib;

private mapping CAPABILITIES_VAR = ([ CAP_AVATAR ]);
private string CMD_IMPORTS_VAR = PlatformBinDir "/avatar/avatar.cmds";

mapping sessions;

protected void setup();
public mixed *try_descend(string session_id);
public void on_descend(string session_id);
protected int add_session(string session_id);
protected int remove_session(string session_id);
public mapping query_sessions();
public string query_username();
public nomask int is_avatar();

/**
 * Initialize AvatarMixin. If this function is overloaded, be advised
 * that the mixin's private variables are initialized in the parent
 * implementation.
 */
protected void setup() {
  CommandGiverMixin::setup();
  SensorMixin::setup();
  ShellMixin::setup();
  sessions = ([ ]);
}

public mixed *try_descend(string session_id) {
  // XXX guard against username conflicts?
  return ({ });
}

public void on_descend(string session_id) {
  add_session(session_id);
  set_homedir(UserDir "/" + query_username());
  set_cwd(query_homedir());
}

protected int add_session(string session_id) {
  sessions += ([ session_id ]); 
  return 1;
}

protected int remove_session(string session_id) {
  sessions -= ([ session_id ]);
  return 1;
}

public mapping query_sessions() {
  return sessions;
}

public string query_user() {
  return reduce(query_sessions(), (: 
    string user_id = SessionTracker->query_user($2);
    return ( ($1 == -1) ? user_id : (($1 == user_id) && user_id) );
  :), -1);
}

/**
 * Return the username associated with this avatar. This name will be
 * consistent across all characters a user plays.
 *
 * @return the username
 */
public string query_username() {
  return UserTracker->query_username(query_user());
}

/**
 * Returns true to designate that this object represents an avatar.
 *
 * @return 1
 */
public nomask int is_avatar() {
  return 1;
}
