/**
 * A base object for living things, e.g. players and NPCs.
 *
 * @author devo@eotl
 * @alias LivingCode
 */

inherit IdMixin;

private string gender;

public string query_gender() {
  return gender;
}

public int set_gender(string g) {
  gender = g;
  return 1;
}

/**
 * Set up a new living object.
 */
public void create() {
  setup_id();
}

/**
 * Returns true to designate that this object is a living character.
 *
 * @return 1
 */
nomask public int is_living() {
  return 1;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 *
 * @return a zero-width mapping of capabilities
 */
public mapping query_capabilities() {
  return IdMixin::query_capabilities();
}
