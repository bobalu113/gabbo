/**
 * Support for objects which can move on their own.
 * 
 * @author devo@eotl
 * @alias MobileMixin
 */

#include <mobile.h>
#include <room.h>

private variables private functions inherit ObjectLib;
private variables private functions inherit FormatStringsLib;

default private variables;

string exit_msgout_fmt,     exit_msgin_fmt, 
       zone_msgout_fmt,     zone_msgin_fmt, 
       teleport_msgout_fmt, teleport_msgin_fmt;

closure exit_msgout,     exit_msgin,
        zone_msgout,     zone_msgin,
        teleport_msgout, teleport_msgin;

nosave object *followers;
nosave mapping following;

default public functions;

/**
 * Move this object through an exit.
 * 
 * @param  verb  the verb causing the movement, infinitive tense 
 *               (e.g. "walk"); should not be null.
 * @param  dir   the direction of the exit being used; must not be null
 * @param  flags some behavior flags
 * @return       1 for success, 0 for failure
 */
int exit(string verb, string dir, int flags) {
  // validate and move
  object here = ENV(THISO);
  object dest = env->load_exit_room(dir);
  if (!objectp(dest)) {
    tell_player(THISP, "Sorry, that area is incomplete.\n");
    return 0;
  }
  int exit_flags = here->query_exit_flags(dir);
  if (exit_flags & EXIT_BLOCKED) {
    tell_player(THISP, "The exit is blocked.\n");
    return 0;
  }
  if ((exit_flags & EXIT_PEDESTRIAN) && THISO->is_vehicle()) {
    tell_player(THISP, "Sorry, pedestrians only.\n");
    return 0;
  }
  if (!move_object(THISO, dest)) {
    tell_player(THISO, "An unseen force prevents you from exiting.\n");
    return 0;
  }

  string here_zone = get_zone(here);
  string dest_zone = get_zone(dest);

  // echo move message
  if (!(flags & MUFFLED_MOVE) && !(exit_flags & EXIT_MUFFLED)) {
    if (here_zone == dest_zone) {
      tell_players(all_inventory(here), (: 
        $1->query_exit_msgout($2, $3) || query_exit_msgout($2, $3) 
      :), here, verb, dir);
      tell_players((all_inventory(dest - ({ THISO })), (: 
        $1->query_exit_msgin($2, $3) || query_exit_msgin($2, $3) 
      :), dest, verb, backdir);
    } else {
      tell_players(all_inventory(here), (: 
        $1->query_exit_msgout($2, $3) || query_zone_msgout($2, $3) 
      :), here, verb, dir);
      tell_players((all_inventory(dest - ({ THISO })), (: 
        $1->query_exit_msgin($2, $3) || query_zone_msgin($2, $3) 
      :), dest, verb, backdir);
    }
  }

  // move followers
  if (!(flags & NO_FOLLOW) && !(exit_flags & EXIT_NO_FOLLOW)) {
    followers -= ({ 0 });
    if (flags & CMD_FOLLOW) {
      foreach (object follower : followers) {
        command(query_command(), follower);
      }
    } else {
      foreach (object follower : followers) {
        follower->exit(verb, dir, flags);
      }
    }
  }

  THISO->set_context("here");
  command("look");
  return 1;
}

/**
 * Teleport this object to another room.
 * 
 * @param  dest  the destination, expressed as an object or string
 * @param  flags some behavior flags
 * @return       1 for success, 0 for failure
 */
int teleport(mixed dest, int flags) {
  if (stringp(dest)) {
    object tmp = find_object(dest);
    if (objectp(tmp)) {
      dest = tmp;
    } else {
      // try loading a new room
      string ret = catch(dest = load_object(dest); publish);
      // now clone it
      if (clonep(dest)) {
        string ret = catch(dest = clone_object(dest); publish);
      }
    }
  } 
  if(!objectp(dest)) {
    return 0;
  }
  object here = ENV(THISO);

  // move us
  if (!move_object(THISO, dest)) {
    return 0;
  }

  // echo move message
  if (!(flags & MUFFLED_MOVE) && ) {
    tell_players(all_inventory(here), (: 
      $1->query_teleport_msgout() || query_teleport_msgout() 
    :), here);
    tell_players((all_inventory(dest - ({ THISO })), (: 
      $1->query_teleport_msgin() || query_teleport_msgin() 
    :), dest);
  }

  // move followers
  followers -= ({ 0 });
  if (flags & CMD_FOLLOW) {
    foreach (object follower : followers) {
      command(query_command(), follower);
    }
  } else if(flags & FOLLOW) {
    foreach (object follower : followers) {
      follower->teleport(dest, flags);
    }
  }

  THISO->set_context("here");
  command("look");
  return 1;
}

/**
 * Return the message displayed to source room upon using an exit within the
 * same zone.
 * 
 * @param  verb the verb causing the movement, infinitive tense 
 *              (e.g. "walk"), should not be null
 * @param  dir  the direction of the exit being used, must not be null
 * @return      the exit message
 */
string query_exit_msgout(string verb, string dir) {
  return funcall(exit_msgout, THISO, verb, dir);
}

/**
 * Set the message displayed to the source room upon using an exit within the
 * same zone, expressed as a format string which will be parsed into a 
 * closure that is run at exit time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_exit_msgout(string fmt) {
  exit_msgout = parse_mobile_format(fmt);
  exit_msgout_fmt = fmt;
  return 1;
}

/**
 * Return the message displayed to destination room upon using an exit within
 * the same zone.
 * 
 * @param  verb the verb causing the movement, infinitive tense 
 *              (e.g. "walk"), should not be null
 * @param  dir  the direction of the exit being used, may be null if the 
 *              'back' direction could not be discerned
 * @return      the entrance message
 */
string query_exit_msgin(string verb, string dir) {
  return funcall(exit_msgin, THISO, verb, dir);
}

/**
 * Set the message displayed to the destination room upon using an exit 
 * within the same zone, expressed as a format string which will be parsed 
 * into a closure that is run at exit time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_exit_msgin(string fmt) {
  exit_msgin = parse_mobile_format(fmt);
  exit_msgin_fmt = fmt;
  return 1;
}

/**
 * Return the message displayed to source room upon using an exit to a 
 * different zone.
 * 
 * @param  verb the verb causing the movement, infinitive tense 
 *              (e.g. "walk"), should not be null
 * @param  dir  the direction of the exit being used, must not be null
 * @return      the exit message
 */
string query_zone_msgout(string verb, string dir) {
  return funcall(zone_msgout, THISO, verb, dir);
}

/**
 * Set the message displayed to the source room upon using an exit to a
 * different zone, expressed as a format string which will be parsed into a 
 * closure that is run at exit time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_zone_msgout(string fmt) {
  zone_msgout = parse_mobile_format(fmt);
  zone_msgout_fmt = fmt;
  return 1;
}

/**
 * Return the message displayed to destination room upon using an exit to a
 * different zone.
 * 
 * @param  verb the verb causing the movement, infinitive tense 
 *              (e.g. "walk"), should not be null
 * @param  dir  the direction of the exit being used, may be null if the 
 *              'back' direction could not be discerned
 * @return      the entrance message
 */
string query_zone_msgin(string verb, string dir) {
  return funcall(zone_msgin, THISO, verb, dir);
}

/**
 * Set the message displayed to the destination room upon using an exit 
 * to a different zone, expressed as a format string which will be parsed 
 * into a closure that is run at exit time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_zone_msgin(string fmt) {
  zone_msgin = parse_mobile_format(fmt);
  zone_msgin_fmt = fmt;
  return 1;
}

/**
 * Return the message displayed to source room when this object is 
 * teleported.
 * 
 * @return      the exit message
 */
string query_teleport_msgout() {
  return funcall(teleport_msgout, THISO);
}

/**
 * Set the message displayed to the source room upon being teleported, 
 * expressed as a format string which will be parsed into a closure that is 
 * run at teleport time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_teleport_msgout(string fmt) {
  teleport_msgout = parse_mobile_format(fmt);
  teleport_msgout_fmt = fmt;
  return 1;
}

/**
 * Return the message displayed to destination room when this object is 
 * teleported.
 * 
 * @return      the entrance message
 */
string query_teleport_msgin() {
  return funcall(teleport_msgin, THISO);
}

/**
 * Set the message displayed to the destination room upon being teleported, 
 * expressed as a format string which will be parsed into a closure that is 
 * run at teleport time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_teleport_msgin(string fmt) {
  teleport_msgin = parse_mobile_format(fmt);
  teleport_msgin_fmt = fmt;
  return 1;
}

/**
 * Return the list of objects, if any, that are following this object.
 * 
 * @return an array of followers
 */
object *query_followers() {
  followers -= ({ 0 });
  return followers;
}

/**
 * Set the list of objects that are following this object.
 * 
 * @param  f an array of followers
 * @return   1 for success, 0 for failure
 */
int set_followers(object *f) {
  // TODO this needs to check for recursive follows
  followers = f;
  foreach (object follower : f) {
    if (!follower->is_following(THISO)) {
      follower->add_following(THISO);
    }
  }
  return 1;
}

/**
 * Add a follower to the list of objects following this object. An object
 * which is already following will be moved to the end of the list.
 * 
 * @param  f the follower to add
 * @return   1 for success, 0 for failure
 */
int add_follower(object f) {
  // TODO this needs to check for recursive follows
  followers -= ({ f });
  followers += ({ f });
  if (!f->is_following(THISO)) {
    f->add_following(THISO);
  }
  return 1;
}

/**
 * Remove a follower from the list of objects following this object.
 * 
 * @param  f the follower to remove
 * @return   1 for success, 0 for failure
 */
int remove_follower(object f) {
  followers -= ({ f });
  if (f->is_following(THISO)) {
    f->remove_following(THISO);
  }
  return 1;
}

/**
 * Find out if an object is a follower of this object.
 * 
 * @param  f the potential follower
 * @return   1 if specified object is following this object, otherwise 0
 */
int is_follower(object f) {
  return (member(followers, f) != -1);
}

/**
 * Return the set of objects, if any, that this object is following.
 * 
 * @return a zero-width mapping of followed objects
 */
mapping query_following() {
  following -= ([ 0 ]);
  return following;
}

/**
 * Set the set of objects that this object is following.
 * 
 * @param  f a zero-width mapping of followed objects
 * @return   1 for success, 0 for failure
 */
int set_following(mapping f) {
  // TODO this needs to check for recursive follows
  following = f;
  foreach (object followee : f) {
    if (!followee->is_follower(THISO)) {
      followee->add_follower(THISO);
    }
  }
  return 1;
}

/**
 * Add an object to the set of objects that this object is following.
 * 
 * @param  f the follower to add
 * @return   1 for success, 0 for failure
 */
int add_following(object f) {
  // TODO this needs to check for recursive follows
  following += ([ f ]);
  if (!f->is_follower(THISO)) {
    f->add_follower(THISO);
  }
  return 1;
}

/**
 * Remove a follower from the list of objects following this object.
 * 
 * @param  f the follower to remove
 * @return   1 for success, 0 for failure
 */
int remove_following(object f) {
  following -= ([ f ]);
  if (f->is_follower(THISO)) {
    f->remove_follower(THISO);
  }
  return 1;
}

/**
 * Find out if an object is being followed by this object.
 * 
 * @param  f the potential followee
 * @return   1 if specified object is followed by this object, otherwise 0
 */
int is_following(object f) {
  return member(following, f);
}

/**
 * Initialize the MobileMixin. This initializes globals and sets the default
 * movement messages.
 */
void setup_mobile() {
  followers = ({ });
  set_exit_msgout(DEFAULT_EXIT_MSGOUT);
  set_exit_msgin(DEFAULT_EXIT_MSGIN);
  set_zone_msgout(DEFAULT_ZONE_MSGOUT);
  set_zone_msgin(DEFAULT_ZONE_MSGIN);
  set_teleport_msgout(DEFAULT_TELEPORT_MSGOUT);
  set_teleport_msgin(DEFAULT_TELEPORT_MSGIN);
}
