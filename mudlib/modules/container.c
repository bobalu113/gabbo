/**
 * A base object for containers.
 * 
 * @author devo@eotl
 * @alias ContainerCode
 */

inherit ThingCode;

/**
 * Returns true to designate that this object is a container.
 * 
 * @return 1 
 */
nomask int is_container() {
  return 1;
}