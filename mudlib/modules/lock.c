/**
 * Support for things which can be locked, like doors and containers.
 * 
 * @author devo@eotl
 * @alias LockMixin
 */

/**
 * Return a zero-width mapping of the capabilities this program provides.
 * 
 * @return a zero-width mapping of capabilities
 */
mapping query_capabilities() {
  return ([ CAP_LOCK ]);
}
