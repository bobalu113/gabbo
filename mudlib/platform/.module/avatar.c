/**
 * A module to support chains of avatars.
 *
 * @author devo@eotl
 * @alias AvatarMixin
 */

inherit CommandGiverMixin;
inherit SensorMixin;
inherit ShellMixin;
inherit SoulMixin;

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
  SoulMixin::setup();
}

public mixed *try_descend(string user_id) {
  return ({ });
}

public void on_descend(mapping session_ids) {
  set_sessions(session_ids);
}

protected void set_sessions(mapping s) {
  sessions = s;
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
