/**
 * A base object for rooms.
 * 
 * @author devo@eotl
 * @alias RoomCode
 */

#include <room.h>

// FUTURE light
// FUTURE searching
// FUTURE color
// TODO object spawning

inherit PropertyMixin;
inherit IdMixin;
inherit DetailMixin;
inherit VisibleMixin;

private variables private functions inherit FormatStringsLib;
private variables private functions inherit ArrayLib;

default private variables;

/** ([ str dir : str dest; int hidden; obj room, 
                 str msgout_fmt, str msgin_fmt ]) **/
mapping exits;

string teleport_msgout_fmt, teleport_msgin_fmt;

/** ([ str format : cl msg ]) */
mapping messages;

default public functions;

/**
 * Return the mapping of all the rooms exists. This is a two-dimensional 
 * mapping, with the exit direction (e.g. "north") as the keys. The first
 * value should be the load name of the room to which the player will be
 * moved when using the exit. The second value is a flag designating whether
 * or not the exit should be displayed when the user looks inside a room.
 * 
 * @return a mapping of the form ([ direction : destination; flags; room ])
 */
mapping query_exits() {
  return exits;
}

/**
 * Set the room's exit mapping.
 * 
 * @param map the mapping of exits to set
 * @return    0 for failure, 1 for success
 * @see       query_exits()
 */
int set_exits(mapping map) {
  if (!mappingp(map)) {
    return 0;
  }
  map = filter(map, (: stringp($1) :));
  exits = map;
  return 1;
}

/**
 * Return the values associated with the specified exit. This returns an 
 * array the same length as the exit mapping's width.
 * 
 * @param  dir the exit direction
 * @return     the exit values
 */
mixed *query_exit(string dir) {
  return m_value(exits, dir);
}

/**
 * Add a new exit, or replace an existing exit.
 * 
 * @param dir    the direction of the exit to add
 * @param dest   the program name of the destination room
 * @param flags  some behavior flags (see <room.h>)
 * @param room   the room this exit leads to, or 0 if the room has not yet
 *               been loaded
 * @return       0 for failure, 1 for success
 * @see          query_exits()
 */
varargs int set_exit(string dir, string dest, int flags, object room,
                     string msgout_fmt, string msgin_fmt) {
  if (!stringp(dir)) {
    return 0;
  }
  m_add(exits, dir, dest, flags, room, msgout_fmt, msgin_fmt);
  return 1;
}

/**
 * Return the destination of a specified exit. This will be either a load
 * name to clone, or an object name of a clone.
 * 
 * @param  dir the direction of the exit
 * @return     the load/object name of the destination, or 0 if no exit 
 *             exists
 */
string query_exit_dest(string dir) {
  return exits[dir, EXIT_DEST];
}

/**
 * Set the destination of a specified exit. This will be either a load name
 * to clone later, or the object name of a previously cloned room.
 * 
 * @param  dir  the direction of the exit
 * @param  dest the exit destination
 * @return      0 for failure, 1 for success
 */
int set_exit_dest(string dir, string dest) {
  if (!stringp(dir)) {
    return 0;
  }
  exits[dir, EXIT_DEST] = dest;
  return 1;
}

/**
 * Get the exit flags of a specfied exit.
 * 
 * @param  dir the direction of the exit
 * @return     the exit flag bitvector
 */
int query_exit_flags(string dir) {
  return exits[dir, EXIT_FLAGS];
}

/**
 * Set the exit flags of a specfied exit.
 * 
 * @param  dir   the direction of the exit
 * @param  flags the exit flag bitvector
 * @return       0 for failure, 1 for success
 */
int set_exit_flags(string dir, int flags) {
  if (!stringp(dir)) {
    return 0;
  }
  exits[dir, EXIT_FLAGS] = flags;
  return 1;
}

/**
 * Get the room this exit is to move the user into. This may be 0 if the
 * room hasn't been loaded yet.
 * 
 * @param  dir the direction of the exit
 * @return     the cloned destination room, or 0 if not yet loaded
 */
object query_exit_room(string dir) {
  return exits[dir, EXIT_ROOM];
}

/**
 * Set the room this exit is to move the user into. This should always be a
 * clone.
 * 
 * @param  dir  the direction of the exit
 * @param  room the cloned destination room
 * @return      0 for failure, 1 for success
 */
int set_exit_room(string dir, object room) {
  if (!stringp(dir)) {
    return 0;
  }
  if (!clonep(room)) {
    return 0;
  }
  exits[dir, EXIT_ROOM] = room;
  return 1;
}

/**
 * Get exit message override for objects exiting this room.
 * 
 * @param  verb the verb causing the movement, infinitive tense 
 *              (e.g. "walk"); should not be null.
 * @param  dir  the direction of the exit being used; must not be null
 * @return      the exit message
 */
string query_exit_msgout(string verb, string dir) {
  if (closurep(exits[dir, EXIT_MSGOUT])) {
    return funcall(exits[dir, EXIT_MSGOUT], verb, dir);
  }
  return 0;
}

/**
 * Set the message override for objects exiting this room, expressed as a 
 * format string which will be parsed into a closure that is run at exit 
 * time to generate the final message.
 * 
 * @param  dir  the direction of the exit being used; must not be null
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_exit_msgout(string dir, string fmt) {
  if (!stringp(dir)) {
    return 0;
  }
  exits[dir, EXIT_MSGOUT_FMT] = fmt;
  if (!closurep(messages[fmt])) {
    messages[fmt] = parse_mobile_format(fmt);
  }
  return 1;
}

/**
 * Get exit message override for objects entering this room.
 * 
 * @param  verb the verb causing the movement, infinitive tense 
 *              (e.g. "walk"); should not be null.
 * @param  dir  the direction of the exit being used; may be null if 'back'
 *              direction could not be discerned
 * @return      the entrance message
 */
string query_exit_msgin(string verb, string dir) {
  if (closurep(exits[dir, EXIT_MSGIN])) {
    return funcall(exits[dir, EXIT_MSGIN], verb, dir);
  }
}

/**
 * Set the message override for objects entering this room, expressed as a 
 * format string which will be parsed into a closure that is run at exit 
 * time to generate the final message.
 * 
 * @param  dir  the direction of the exit being used; must not be null
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_exit_msgin(string dir, string fmt) {
  if (!stringp(dir)) {
    return 0;
  }
  exits[dir, EXIT_MSGIN_FMT] = fmt;
  if (!closurep(messages[fmt])) {
    messages[fmt] = parse_mobile_format(fmt);
  }
  return 1;
}

/**
 * Get exit message override for objects teleporting out of this room.
 * 
 * @return      the exit message
 */
string query_teleport_msgout() {
  if (closurep(teleport_msgout)) {
    return funcall(teleport_msgout);
  }
  return 0;
}

/**
 * Set the message override for objects teleporting out of this room, 
 * expressed as a format string which will be parsed into a closure that is 
 * run at exit time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_teleport_msgout(string fmt) {
  if (!stringp(dir)) {
    return 0;
  }
  exits[dir, TELEPORT_MSGOUT_FMT] = fmt;
  if (!closurep(messages[fmt])) {
    messages[fmt] = parse_mobile_format(fmt);
  }
  return 1;
}

/**
 * Get exit message override for objects teleporting into this room.
 * 
 * @return      the entrance message
 */
string query_teleport_msgin() {
  if (closurep(teleport_msgin)) {
    return funcall(teleport_msgin);
  }
  return 0;
}

/**
 * Set the message override for objects teleporting into this room, expressed 
 * as a format string which will be parsed into a closure that is run at exit 
 * time to generate the final message.
 * 
 * @param  fmt the format string
 * @return     1 for success, 0 for failure
 */
int set_teleport_msgin(string fmt) {
  if (!stringp(dir)) {
    return 0;
  }
  exits[dir, TELEPORT_MSGOUT_FMT] = fmt;
  if (!closurep(messages[fmt])) {
    messages[fmt] = parse_mobile_format(fmt);
  }
  return 1;
}

/**
 * {@inheritDoc}
 */
public void create() {
  setup_property();
  setup_id();
  setup_detail();
  setup_visible();
  set_primary_id("here");
  add_secondary_id("here");
  exits = m_allocate(0, 7);
  messages = m_allocate(0, 1);
}

/**
 * Returns true to designate that this object represents a room in the game.
 * 
 * @return 1 
 */
nomask public int is_room() {
  return 1;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 * 
 * @return a zero-width mapping of capabilities
 */
mapping query_capabilities() {
  return PropertyMixin::query_capabilities()
             + IdMixin::query_capabilities()
         + DetailMixin::query_capabilities()
        + VisibleMixin::query_capabilities();
}
