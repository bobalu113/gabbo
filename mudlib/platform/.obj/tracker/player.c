/**
 * The PlayerTracker keeps track of the game's players. The relationship of
 * users to players is currently one to one, but could be easily changed to
 * one to many or many to many.
 *
 * @author devo@eotl
 * @alias PlayerTracker
 */

inherit PlayerLib;

// ([ str player_id : PlayerInfo ])
mapping players;
// ([ str user_id : ([ str player_id, ...]) ])
mapping users;
int player_counter;

string new_player(string user_id);
mapping query_players(string user_id);
string query_last_session(string player_id);
string generate_id();

string new_player(string user_id) {
  object logger = LoggerFactory->get_logger(THISO);
  string player_id = generate_id();
  players[player_id] = (<PlayerInfo> 
    id: player_id,
    user: user_id
  );
  if (!member(users, user_id)) {
    users[user_id] = ([ ]);
  }
  users[user_id] += ([ player_id ]);
  return player_id;
}

mapping query_players(string user_id) {
  return users[user_id];
}

string query_last_session(string player_id) {
  if (!member(players, player_id)) {
    return 0;
  }
  return players[player_id]->last_session;
}

string generate_id() {
  return sprintf("%s#%d", program_name(THISO), ++player_counter);
}

int create() {
  players = ([ ]);
  users = ([ ]);
  return 0;
}