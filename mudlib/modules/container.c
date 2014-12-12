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