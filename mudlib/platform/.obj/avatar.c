/**
 * Avatar object for the platform zone. This will be the super-avatar to all
 * flavor avatars.
 *
 * @author devo@eotl
 * @alias PlatformAvatar
 */

inherit AvatarMixin;
inherit SoulMixin;

#define WORKROOM   "workroom"

protected void setup() {
  AvatarMixin::setup();
  SoulMixin::setup();
  return;
}

mixed *try_descend(string session_id) {
  mixed *result = AvatarMixin::try_descend(session_id);
  string user_id = SesssionTracker->query_user(session_id);
  string username = UserTracker->query_username(user_id);

  string player_id = get_player(user_id);
  if (!player_id) {
    throw((<Exception> 
      message: sprintf("no player found for user %O %O", user_id, username)
    ));
  }
  
  object avatar, room;
  string subsession_id = PlayerTracker->get_last_session(player_id);

  if (subsession_id 
      && is_active(subsession_id)
      && is_subsession(session_id, subsession_id)) {
    // use avatar and room from session already in progress
    avatar = SessionTracker->query_avatar(subsession_id);
    room = ENV(avatar);
  } else {
    // load start room
    string start_room = get_start_room(player_id, username);
    room = load_start_room(start_room, username);
    if (!room) {
      throw((<Exception> 
        message: sprintf("unable to load start room %O %O", 
                         player_id, start_room)
      ));
    }

    // clone new avatar
    string avatar_path = get_avatar_path(room, player_id);
    avatar = clone_object(avatar_path);
    if (!avatar) {
      throw((<Exception> 
        message: sprintf("unable to clone avatar %O %O", 
                         player_id, avatar_path)
      ));
    }

    // start new session
    subsession_id = SessionTracker->new_session(user_id, session_id);
    if (!subsession_id) {
      throw((<Exception> 
        message: sprintf("failed to start player session %O %O", 
                         user_id, session_id)
      ));
    }
    SessionTracker->set_avatar(subsession_id, avatar);
  }

  // avatar->try_descend
  mixed *args, ex;
  if (ex = catch(args = avatar->try_descend(subsession_id))) {
    logger->warn("caught exception in try_descend: %O", ex);
    result = ({ 0, 0, 0 }) + result;
  else {
    result = ({ subsession_id, player_id, room }) + result;
  }

  return result;
}

void on_descend(string session_id, string subsession_id, string player_id, 
                object room, varargs mixed *args) {
  AvatarMixin::on_descend(session_id);

  object avatar = SessionTracker->query_avatar(subsession_id);  
  if (avatar) {
    if (!SessionTracker->resume_session(subsession_id)) {
      logger->warn("failed to resume player session: %O %O", 
                   player_id, subsession_id);
      return;
    }
    apply(#'call_other, avatar, "on_descend", 
          subsession_id, room, player_id, args);
  }

  return;
}

string get_player(string user_id) {
  mapping player_ids = PlayerTracker->query_players(user_id);
  if (!player_ids || !sizeof(player_ids)) {
    return PlayerTracker->new_player(user_id);
  } else {
    return m_indices(player_ids)[0];
  }
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

string get_avatar_path(object room, string player_id) {
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
  return avatar_path;
}

string get_default_start_room(string username) {
  return sprintf("%s/%s/%s.c", UserDir, username, WORKROOM);
}
