/**
 * A base object for rooms.
 * 
 * @author devo@eotl
 * @alias RoomCode
 */

// FUTURE light
// FUTURE searching
// FUTURE color
// TODO object spawning

inherit IdMixin;
inherit DetailMixin;
inherit PropertyMixin;

default private variables;

string name;
string description;

/** ([ str dir : str path; int hidden ]) **/
mapping exits;

default public functions;

/**
 * Return the name of the room. Formerly called the "short description", this
 * name will be shown to the user everytime they enter the room. It should be
 * short and recognizable.
 * 
 * @return the name of the room
 */
string query_name() { 
  return name;
}

/**
 * Return a long description of the room. This will be displayed to the 
 * player when they explicitly look inside a room. It can be as long as is
 * needed, but probably shouldn't take up more than a page of a user's 
 * screen.
 * 
 * @return the long description of the room
 */
string query_description() { 
  return description;
}

/**
 * Return the mapping of all the rooms exists. This is a two-dimensional 
 * mapping, with the exit direction (e.g. "north") as the keys. The first
 * value should be the program name of the room to which the player will be
 * moved when using the exit. The second value is a flag designating whether
 * or not the exit should be displayed when the user looks inside a room.
 * 
 * @return a mapping of the form ([ direction : destination; hidden ])
 */
mapping query_exits() {
  return exits;
}

/**
 * Return the destination of a particular exit. This will be the program name
 * of the room to which the exit takes the user.
 * 
 * @param  dir the direction of the exit
 * @return     the program name of the destination, or 0 if no exit exists
 */
string query_exit(string dir) {
  return exits[dir, 0];
}

/**
 * Test whether a given exit is hidden.
 * 
 * @param  dir the direction of the exit to test
 * @return     1 if the exit is hidden, otherwise 0
 */
int is_exit_hidden(string dir) {
  return exits[dir, 1];
}

/**
 * Set the name of the room. S
 * 
 * @param str the name to set
 * @return    0 for failure, 1 for success
 * @see       query_name()
 */
int set_name(string str) {
  name = str;
  return 1;
}

/**
 * Set the room's description. 
 * 
 * @param str the description to set
 * @return    0 for failure, 1 for success
 * @see       query_description()
 */
int set_description(string str) {
  description = str;
  return 1;
}

/**
 * Set the room's exit mapping.
 * 
 * @param map the mapping of exists to set
 * @return    0 for failure, 1 for success
 * @see       query_exits()
 */
int set_exits(mapping map) {
  exits = map;
  return 1;
}

/**
 * Add a new exit, or replace an existing exit.
 * 
 * @param dir    the direction of the exit to add
 * @param dest   the program name of the destination room
 * @param hidden 1 if the exit should be hidden, otherwise 0
 * @return       0 for failure, 1 for success
 * @see          query_exits()
 */
varargs int add_exit(string dir, string dest, int hidden) {
  exits += ([ dir : dest; hidden ]);
  return 1;
}

/**
 * {@inheritDoc}
 */
protected void setup_id() {
  IdMixin::setup_id();
  set_primary_id("here");
}

/**
 * Returns true to designate that this object represents a room in the game.
 * 
 * @return 1 
 */
nomask int is_room() {
  return 1;
}
