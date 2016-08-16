/**
 * Avatar object for the platform zone. This will be the super-avatar to all
 * flavor avatars.
 *
 * @author devo@eotl
 * @alias PlatformAvatar
 */

inherit AvatarMixin;
inherit CommandGiverMixin;

#define WORKROOM   "workroom"

mixed *try_descend(string user_id, object login) {
  mixed *result = AvatarMixin::try_descend(user_id, login);
  string username = UserTracker->query_username(user_id);
  string player_id = get_player(user_id, username);
  if (!player_id) {
    throw((<Exception> 
      message: sprintf("player %O for user %O not found", username, user_id)
    ));
  }
  string start_room = get_start_room(player_id, username);

  object room = load_start_room(start_room, username);
  if (!room) {
    throw((<Exception> 
      message: sprintf("unable to load start room %O", start_room)
    ));
  }

  string zone = get_zone(room);
  string flavor = ZoneTracker->query_flavor(zone);
  string avatar_path = FlavorTracker->query_avatar(flavor, player_id);
  


  // clone flavor avatar
  
  // avatar->try_descend
  result = ({ avatar });
  return result;
}

void on_descend(string session_id, object login, object avatar) {
  // ::on_descend
  // avatar->descend_signal
  //   move to start room
  //   restore inventory 
  //   `sense here`  
}

string get_player(string user_id, string username) {
  string result;
  string result = PlayerTracker->query_player_id(user_id, username);
  if (!result) {
    result = PlayerTracker->new_player(user_id, username);
  }
  return result;
}

string get_start_room(string player_id, string username) {
  string result;
  string last_session = PlayerTracker->query_last_session(player_id);
  if (last_session) {
    result = SessionTracker->query_last_room(last_session);
  } else {
    result = get_default_start_room(username);
  }
  return result;
}

object load_start_room(string room, string username) {
  object result = load_object(name);
  if (!result) {
    string default_room = get_default_start_room(username);
    if (default_room != room) {
      result = load_object(default_room);
    }
  }
  return result;
}

string get_default_start_room(string username) {
  return sprintf("%s/%s/%s.c", UserDir, username, WORKROOM);
}
