/**
 * The simulated efun object.
 *
 * @author devo@eotl
 * @alias SimulEfunObject
 */

// ([ username : active_avatar ])
mapping players;

object find_player(string username) {
  return players[username];
}

int move_resolved(mixed item, mixed dest) {
  move_object(item, dest);
  if (!objectp(item)) { item = find_object(item); }
  if (!objectp(dest)) { dest = find_object(dest); }
  return (item && dest && (environment(item) == dest));
}
