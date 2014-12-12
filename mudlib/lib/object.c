/**
 * Utility library for dealing with objects.
 * 
 * @author devo@eotl
 * @alias ObjectLib
 */
#include <sys/files.h>

/**
 * Test whether an object is reachable from another object.
 * 
 * @param  ob  the object trying to be accessed
 * @param  who the object doing the accessing, defaults to THISP
 * @return     1 if the object is reachable, otherwise 0
 */
varargs int is_reachable(object ob, object who) {
  if (!who) {
    who = THISP;
  }
  return (ob == who) || (ob == ENV(who));
}