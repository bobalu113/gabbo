/**
 * A base object for non-player characters.
 * 
 * @author devo@eotl
 * @alias CreatureCode
 */

inherit LivingCode;

/**
 * Returns true to designate that this object is a non-player character.
 * 
 * @return 1 
 */
nomask int is_creature() {
  return 1;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 * @return a zero-width mapping of capabilities
 */
mapping query_capabilities() {
  return LivingCode::query_capabilities();
}
