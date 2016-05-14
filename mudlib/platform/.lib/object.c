/**
 * Utility library for dealing with objects.
 *
 * @author devo@eotl
 * @alias ObjectLib
 */

#include <sys/files.h>
#include <sys/inherit_list.h>
#include <object.h>

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

string get_create_uid(string objectname) {
  if (objectp(DomainTracker)) {
    return DomainTracker->query_domain_id(objectname);
  } else {
    return DEFAULT_DOMAIN; // XXX gaping security hole
  }
}

string get_user(object ob) {
  string euid = geteuid(ob);
  string result;
  int pos = member(euid, '@');
  if (pos != -1) {
    result = euid[0..(pos - 1)];
  }
  return result;
}

mixed *get_path_info(mixed ob) {
  string oname, uid, user;
  if (objectp(ob)) {
    oname = object_name(ob);
    uid = getuid(ob);
    user = get_user(ob);
  } else if (stringp(ob)) {
    oname = ob;
    uid = get_create_uid(ob);
  }
  if (oname[<2..<1] == ".c") {
    oname = oname[0..<3];
  }

  string zone, category, file;
  int clone;

  string *parts = explode(oname, "/");
  int len = sizeof(parts);
  int i = 0;
  do {
    if (parts[i][0] == '.') {
      if (i == 0) {
        zone = "";
        category = implode(parts[i..<2], ".");
      } else {
        zone = implode(parts[0..<(i - 1)], ".");
        category = implode(parts[i..<2], ".")[1..];
      }
      break;
    }
  } while (++i < (len - 1));
  if (!zone) {
    zone = implode(parts[0..<2], ".");
    category = "";
  }

  parts = explode(parts[<1], "#");
  file = parts[0];
  if (sizeof(parts) >= 2) {
    file = parts[0];
    clone = to_int(parts[<1]);
  }

  string domain = DEFAULT_DOMAIN; // XXX gaping security hole
  if (objectp(DomainTracker)) {
    domain = DomainTracker->query_domain_id(oname);
  }

  return ({ oname, uid, user, domain, zone, category, file, clone });
}

/**
 * Get the zone an object belongs to.
 *
 * @param  ob the object to query
 * @return    the object's zone
 */
string get_zone(object ob) {
  return get_path_info(ob)[PATH_INFO_ZONE];
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
