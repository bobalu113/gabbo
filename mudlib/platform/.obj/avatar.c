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
  mixed *result = AvatarMixin::try_descend(user_id);
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

  // clone flavor-specific avatar
  string zone = get_zone(room);
  string flavor = ZoneTracker->query_flavor(zone);
  if (!flavor) {
    throw((<Exception> 
      message: sprintf("unable to deterimine avatar flavor for zone %O", zone)
    ));    
  }
  string avatar_path = FlavorTracker->query_avatar(flavor, player_id);
  if (!avatar_path) {
    throw((<Exception> 
      message: sprintf("unable to deterimine avatar path for flavor %O", 
                       flavor)
    ));    
  }
  object avatar = clone_object(avatar_path);
  
  // avatar->try_descend
  mixed *args, ex;
  if (ex = catch(args = avatar->try_descend(user_id, THISO))) {
    logger->warn("caught exception in try_descend: %O", ex);
    result = ({ 0, 0, 0 }) + result;
  else {
    result = ({ avatar, player_id, room }) + result;
  }

  return result;
}

void on_descend(string session_id, object login, object avatar, 
                object room, varargs mixed *args) {
  AvatarMixin::on_descend(session_id);

  if (avatar) {
    string subsession_id = start_session(avatar, session_id);
    avatar->on_descend(subsession_id, player_id, room, args)
  }

  return;
}

string start_session(object avatar, string session_id) {
  /* WTF only register can set connection (only platform avatar)? 
  if (!switch_connection(THISO, avatar);
    logger->warn("failed to switch connection from platform avatar to "
                 "flavor avatar: %O %O", THISO, avatar);
    return;
  }
  */
  string user_id = SessionTracker->query_user(session_id)
  string subsession_id = SessionTracker->new_session(user_id, session_id);
  if (!subsession_id) {
    logger->warn("failed to start session: %O", user_id);
    return;
  }
  if (!UserTracker->session_start(user_id, subsession_id)) {
    logger->warn("failed to attach user session: %O %O", 
                 user_id, subsession_id);
    return;
  }
  return subsession_id;
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
