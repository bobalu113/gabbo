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

protected void setup() {
  AvatarMixin::setup();
  SoulMixin::setup();
}

/**
 * Returns true to designate that this object represents a player.
 *
 * @return 1
 */
public nomask int is_player() {
  return 1;
}
