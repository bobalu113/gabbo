/**
 * A base object for containers.
 * 
 * @author devo@eotl
 * @alias ContainerCode
 */

inherit ThingCode;

public void create() {
  ThingCode::create();
}

/**
 * Returns true to designate that this object is a container.
 * 
 * @return 1 
 */
nomask public int is_container() {
  return 1;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 * 
 * @return a zero-width mapping of capabilities
 */
public mapping query_capabilities() {
  return ThingCode::query_capabilities();
}
