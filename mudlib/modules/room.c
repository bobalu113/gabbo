/**
 * Top-level Room Module.
 * 
 * @author devo@eotl
 * @alias RoomCode
 */

// FUTURE light
// FUTURE searching
// TODO object spawning
// TODO details (descs)

private string name;
private string description;

// ([ dir : path; hidden ])
private mapping exits;

void set_name(string str) {
  name = str;
}

string query_name() { 
  return name;
}

void set_description(string str) {
  description = str;
}

string query_description() { 
  return description;
}

void set_exits(mapping map) {
  exits = map;
}

varargs void add_exit(string dir, string dest, int hidden) {
  exits += ([ dir : dest; hidden ]);
}

mapping query_exits() {
  return exits;
}

string query_exit(string dir) {
  return exits[dir, 0];
}

int is_exit_hidden(string dir) {
  return exits[dir, 1];
}
