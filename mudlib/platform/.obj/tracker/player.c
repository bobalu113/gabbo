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
// ([ str user_id : ([ str name : str player_id, ...]) ])
mapping users;
int player_counter;

string new_player(string user_id, string name) {
  object logger = LoggerFactory->get_logger(THISO);
  string player_id = generate_id();
  users[player_id] = (<UserInfo> 
    id: player_id,
    name: name
  );
  if (!member(users, user_id)) {
    users[user_id] = ([ ]);
  }
  if (member(users[user_id], name)) { 
    logger->warn("failed to create player for %O: player %O already exists", 
                 user_id, name);
    return 0;
  }
  users[user_id] += ([ name : player_id ]);
  return player_id;
}

string generate_id() {
  return sprintf("%s#%d", program_name(THISO), ++player_counter);
}