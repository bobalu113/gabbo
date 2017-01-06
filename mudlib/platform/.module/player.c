/**
 * A module to support player objects.
 *
 * @author devo@eotl
 * @alias PlayerMixin
 */
#include <capabilities.h>

inherit AvatarMixin;
inherit SoulMixin;

private mapping CAPABILITIES_VAR = ([ CAP_PLAYER ]);

string player;

protected void setup();
protected void set_player(string player_id);
public string query_player();
public nomask int is_player();

/**
 * Setup the PlayerMixin.
 */
protected void setup() {
  AvatarMixin::setup();
  SoulMixin::setup();
}

/**
 * Set the player id of this player object.
 * 
 * @param player_id the player id
 */
protected void set_player(string player_id) {
  player = player_id;
}

/**
 * Query the player id of this player object.
 * 
 * @return the player id
 */
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
