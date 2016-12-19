/**
 * A module to support player objects.
 *
 * @author devo@eotl
 * @alias PlayerMixin
 */

inherit AvatarMixin;
inherit SoulMixin;

mapping CAPABILITIES_VAR = ([ CAP_PLAYER ]);
string CMD_IMPORTS_VAR = ([ ]);


string player;

protected void set_player(string player_id);
public string query_player();

protected void setup() {
  AvatarMixin::setup();
  SoulMixin::setup();
}

protected void set_player(string player_id) {
  player = player_id;
}

public string query_player() {
  return player;
}

/**
 * Returns true to designate that this object represents a player.
 *
 * @return 1
 */
public nomask int is_player() {
  return 1;
}
