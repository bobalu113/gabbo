/**
 * A module to support chains of avatars.
 *
 * @author devo@eotl
 * @alias AvatarMixin
 */

inherit CommandGiverMixin;
inherit SensorMixin;
inherit ShellMixin;

mapping CAPABILITIES_VAR = ([ CAP_AVATAR ]);
string CMD_IMPORTS_VAR = AvatarBinDir "/avatar.xml";

mapping sessions;

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
  return ({ });
}

public void on_descend(string session_id) {
  add_session(session_id);
}

protected int add_session(string session_id) {
  sessions += ([ session_id ]); 
  return 1;
}

protected int remove_session(string session_id) {
  sessions -= ([ session_id ]);
  return 1;
}

public string query_sessions() {
  return sessions;
}

/**
 * Returns true to designate that this object represents an avatar.
 *
 * @return 1
 */
public nomask int is_avatar() {
  return 1;
}
