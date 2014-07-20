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
