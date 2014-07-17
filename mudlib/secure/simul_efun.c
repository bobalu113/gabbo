
// ([ username : active_avatar ])
mapping players;

object find_player(string username) {
  return players[username];
}
