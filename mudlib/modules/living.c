/**
 * A base object for living things, e.g. players and NPCs.
 * 
 * @author devo@eotl
 * @alias LivingCode
 */

inherit IdMixin;

/**
 * Set up a new living object.
 */
public void create() {
  IdMixin::setup_id();
}

/**
 * Returns true to designate that this object is a living character.
 * 
 * @return 1 
 */
nomask public int is_living() {
  return 1;
}
