/**
 * Utility library for dealing with objects.
 *
 * @author devo@eotl
 * @alias ObjectLib
 */

#include <sys/files.h>
#include <sys/inherit_list.h>

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
  return (ob == who) || (ob == ENV(who)) || (ENV(ob) == who);
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
 * Test if an object has a specified capability.
 *
 * @param  ob  the object to test
 * @param  cap the capability in question
 * @return     1 if the object has the specified capability, otherwise 0
 */
int is_capable(object ob, string cap) {
  mapping caps = query_capabilities(ob);
  return (member(caps, cap));
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

/**
 * Is the object diegetic? Diegetic objects are the people, places, and
 * things that comprise the game world. Non-diegetic objects would be things
 * like command objects, service objects, or libraries.
 *
 * @param  ob the object to test
 * @return    1 if the object is diegetic, otherwise 0
 */
int is_diegetic(object ob) {
  return ob->is_stuff() || ob->is_room();
}

/**
 * Return the objective pronoun for a given object.
 *
 * @param  what the object
 * @return      "he, "she", or "it", depending on gender
 */
string objective(object what) {
  switch (what->query_gender()) {
    case "male": return "he";
    case "female": return "she";
  }
  return "it";
}

int send_prompt(object who) {
  mixed prompt = set_prompt(0, who);
  if (closurep(prompt)) {
    efun::tell_object(who, funcall(prompt));
  } else {
    efun::tell_object(who, prompt);
  }
  return 1;
}
