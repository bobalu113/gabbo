/**
 * The simulated efun object.
 *
 * @author devo@eotl
 * @alias SimulEfunObject
 */

// ([ username : active_avatar ])
mapping players;

object find_player(string username) {
  //return players[username];
  object *u = filter(users(), (: $1->query_username() == $2 :), username);
  if (sizeof(u)) {
    return u[0];
  } else {
    return 0;
  }
}

int move_resolved(mixed item, mixed dest) {
  move_object(item, dest);
  if (!objectp(item)) { item = find_object(item); }
  if (!objectp(dest)) { dest = find_object(dest); }
  return (item && dest && (environment(item) == dest));
}

nomask void write(mixed msg) {
  raise_error("write is disabled() except for privileged objects");
}
