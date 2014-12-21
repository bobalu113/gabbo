/**
 * Utility library for dealing with objects.
 * 
 * @author devo@eotl
 * @alias ObjectLib
 */

#include <sys/files.h>

private int valid_environment(object arg);
private object find_room(object arg);

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
string get_zone(object ob) {
  return "zone";
}

/**
 * Tell a list of players a message, setting THISP to the target player each
 * time the message is built. This message must be expressed a closure which 
 * evaluates to a string. Extra args will be passed to the closure if 
 * specified. At the end of this routine, THISP will be set back to its 
 * original value.
 * 
 * @param players an array of players to message
 * @param msg     either a string, or a closure which evaluates to a string
 * @param args    if msg is a string, args will be passed as sprintf() args;
 *                if msg is a closure, extra args to be passed to the closure
 */
varargs void tell_players(object *players, mixed msg, varargs mixed *args) {
  object oldp = THISP;
  if (stringp(msg)) {
    msg = (: apply(#'sprintf, $1, $2) :);
  }
  foreach (object player : players) {
    if (living(player)) {
      set_this_player(player);
      string str;
      if (stringp(msg)) {
        str = funcall((: apply(#'sprintf, $1, $2) :), msg, args);
      } else {
        str = apply(msg, args);
      }
      tell_object(player, str);
    }
  }
  set_this_player(oldp);
  return;
}

/**
 * A convenience method for passing a single player to tell_players().
 * 
 * @param player the player to tell to
 * @param msg     either a string, or a closure which evaluates to a string
 * @param args    if msg is a string, args will be passed as sprintf() args;
 *                if msg is a closure, extra args to be passed to the closure
 */
varargs void tell_player(object player, mixed msg, varargs mixed *args) {
  apply(#'tell_players, ({ player }), msg, args);
  return;
}

/**
 * Test if an object has a specified capability.
 * 
 * @param  ob  the object to test
 * @param  cap the capability in question
 * @return     1 if the object has the specified capability, otherwise 0
 */
int is_capable(object ob, string cap) {
  mapping m = ob->query_capabilities();
  return (mappingp(m) && member(m, cap));
}

/**
 * Return the display name for a specified object.
 * 
 * @param  ob the object to display
 * @return    the object's display name
 */
string get_display(object ob) {
  return ob->query_name() || ob->query_short();
}
