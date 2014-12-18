/**
 * A base object for inanimate objects.
 * 
 * @author devo@eotl
 * @alias ThingCode
 */

inherit IdMixin;

/**
 * Set up a new thing.
 */
public void create() {
  setup_id();
}

/**
 * Boilerplate id() implementation
 * @param  str the potential id
 * @return     non-zero if the id matches
 */
int id(string str) {
  return test_id(str);
}

/**
 * Returns true to designate that this object represents a thing in the game.
 * 
 * @return 1 
 */
nomask public int is_thing() {
  return 1;
}