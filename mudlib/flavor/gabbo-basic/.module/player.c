/**
 * A module to support player characters.
 *
 * @author devo@eotl
 * @alias PlayerMixin
 */

inherit NameMixin;
inherit LiteracyMixin;
inherit SpeechMixin;

mapping CAPABILITIES_VAR = ([ CAP_PLAYER ]);

/**
 * Initialize AvatarMixin. If this function is overloaded, be advised
 * that the mixin's private variables are initialized in the parent
 * implementation.
 */
protected void setup() {
  NameMixin::setup();
  LiteracyMixin::setup();
  SpeechMixin::setup();
}

/**
 * Returns true to designate that this object represents a player character.
 *
 * @return 1
 */
public nomask int is_player() {
  return 1;
}
