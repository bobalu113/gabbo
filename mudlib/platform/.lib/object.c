/**
 * Utility library for dealing with objects.
 *
 * @author devo@eotl
 * @alias ObjectLib
 */

#include <sys/files.h>
#include <sys/inherit_list.h>
#include <sys/functionlist.h>
#include <object.h>
#include <zone.h>

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
  int tmp;
  if (objectp(ob)) {
    oname = object_name(ob);
    uid = getuid(ob);
    user = get_user(ob);
  } else if (stringp(ob)) {
    oname = ob;
    uid = get_create_uid(ob);
  }
  tmp = strlen(oname);
  if ((tmp >= 2) && (oname[<2..<1] == LPC_EXTENSION)) {
    if (tmp) {
      oname = oname[0..<3];
    } else {
      oname = "";
    }
  }

  string zone, category, file;
  int clone;

  string *parts = explode(oname, "/");
  int len = sizeof(parts);
  int i = 0;
  while (i < (len - 1)) {
    if (strlen(parts[i]) && (parts[i][0] == CATEGORY_DELIM[0])) {
      if (i == 0) {
        zone = "";
        category = implode(parts[i..<2], CATEGORY_DELIM);
      } else {
        zone = implode(parts[0..(i - 1)], ZONE_DELIM);
        category = implode(parts[i..<2], CATEGORY_DELIM)[1..];
      }
      break;
    }
    i++;
  }
  if (!zone) {
    zone = implode(parts[0..<2], ZONE_DELIM);
    category = "";
  }

  parts = explode(parts[<1], CLONE_DELIM);
  file = parts[0];
  if (sizeof(parts) >= 2) {
    clone = to_int(parts[<1]);
  }

  string domain = DEFAULT_DOMAIN; // XXX gaping security hole
  if (objectp(DomainTracker)) {
    domain = DomainTracker->query_domain_id(oname);
  }

  return ({ oname, uid, user, domain, zone, category, file, clone });
}

/**
 * Get the flavor the object belongs to.
 *
 * @param ob the object to query
 * @return   the object's flavor
 */
string get_flavor(object ob) {
  string name = program_name(ob);
  if (name[0..12] == "/platform/") {
    return "platform";
  }
  return "";
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

varargs object reload_object(mixed ob, int flags) {  
  // TODO move/restore inventory
  // TODO move/restore shadows

  string name;
  if (stringp(ob)) {
    name = ob;
    ob = 0;
  } else if (objectp(ob)) {
    name = load_name(ob);
  } else {
    return 0;
  }

  // save variable values
  mixed *vars;
  if (ob) {
    vars = variable_list(ob, RETURN_FUNCTION_NAME|RETURN_VARIABLE_VALUE);
  }

  // TODO reload inherited programs
  // reload the blueprint, and clone if necessary
  object result;
  if (ob) {
    if (clonep(ob)) {
      if (flags & RELOAD_RELOAD_BLUEPRINT) {
        object blueprint = blueprint(ob);
        if (blueprint) {
          destruct(blueprint);
        }
      }
      if (ob) {
        destruct(ob);
      }
      result = clone_object(name);
    } else {
      destruct(ob);
      result = load_object(name);
    }
  } else {
    result = load_object(name);
  }
  
  // restore variables
  if (vars) {
    for (int i = 0, int j = sizeof(vars); i < j; i += 2) {
      closure var = funcall(bind_lambda(#'symbol_variable, result), vars[i]);
      closure setter = bind_lambda(unbound_lambda(({ 'val }),
        ({ #'=, var, 'val })
      ), result);
      funcall(setter, vars[i + 1]);
    }    
  }

  return result;
}
