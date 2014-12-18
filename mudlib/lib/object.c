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

/**
 * Get the zone an object belongs to.
 * 
 * @param  ob the object to query
 * @return    the object's zone
 */
string query_zone(object ob) {
  return "";
}

/**
 * Tell a list of players a message, setting THISP to the target player each
 * time the message is built. This message must be expressed a closure which 
 * evaluates to a string. Extra args will be passed to the closure if 
 * specified. At the end of this routine, THISP will be set back to its 
 * original value.
 * 
 * @param players an array of players to message
 * @param cl      a closure which evaluates to a string
 * @param args    extra args to be passed to the closure
 */
varargs void tell_players(object *players, closure cl, varargs mixed *args) {
  object oldp = THISP;
  foreach (object player : players) {
    if (is_living(player)) {
      set_this_player(player);
      string msg = apply(cl, args);
      tell_object(player, msg);
    }
  }
  set_this_player(oldp);
  return;
}
