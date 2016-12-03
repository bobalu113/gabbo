/**
 * A standard avatar for player characters.
 *
 * @author devo@eotl
 * @alias Avatar
 */
#include <sys/functionlist.h>

inherit HumanCode;
inherit PlayerMixin;
inherit AvatarMixin;

inherit ConnectionLib;

#define UserCommandSpec  (query_homedir() + _EtcDir "/commands.xml")
#define DescendScript    (query_homedir() + _BinDir "/descend.cmd")

string player;

public string query_username();
protected void set_player(string player_id);
public string query_player();

/**
 * Invoked by the login object to set up a newly spawned avatar. At the time
 * this lfun is called, interactivity has not yet been transfered to the
 * avatar. It also has not yet been moved to the player's starting location.
 * @param  username the username to which this avatar belongs
 */
protected void setup() {
  PlayerMixin::setup();
  AvatarMixin::setup();
}

#ifdef STATEOB

private void save_state() {
  object logger = LoggerFactory->get_logger(THISO);
  mapping out = ([ ]);
  mixed *vars = variable_list(THISO,
                              RETURN_FUNCTION_NAME
                            | RETURN_FUNCTION_FLAGS
                            | RETURN_VARIABLE_VALUE);
  for (int i = 0, int j = sizeof(vars); i < j; i += 3) {
    string name = vars[i];
    int type = vars[i + 1];
    int value = vars[i + 2];
    string flavor = get_flavor(variable_exists(name, THISO, NAME_HIDDEN))
    if (type & TYPE_MOD_NOSAVE) {
      continue;
    }
    if (!out[flavor]) {
      out[flavor] = ([ ]);
    }
    out[flavor][name] = value;
  }
  foreach (string flavor, mapping vals : out) {
    object state_obj;
    string err;
    string state_prg = sprintf(StateObjDir "/avatar/%s-%d.c",
                               flavor, program_time());
    if (!file_exists(state_prg)) {
      write_state_program(state_prg, vals);
    }
    if (err = catch(state_obj = clone_object(state_prg))) {
      logger->error("Couldn't clone state object: %s, %s", state_prg, err);
      continue;
    }
    foreach (string var, mixed val : vals) {
      state_obj->set_variable(var, val);
    }
    string savefile = sprintf("/state/player/%s/%s.val",
                              query_username(), flavor);
    if (!state_obj->save_state(savefile)) {
      logger->error("Couldn't save state: %s", savefile);
    }
    destruct(state_obj);
  }
  return;
}

private void restore_state() {
  mapping in = ([ ]);
  string *ancestors = inherit_list();
  for (int i = sizeof(ancestors) - 1; i >= 0; i--) {
    string flavor = get_flavor(ancestors[i]);
    if (!in[flavor]) {
      in[flavor] = ([ ]);
    }
    string savefile = sprintf("/state/player/%s/%s.val",
                              query_username(), flavor);
    in[flavor] += restore_value(read_file(savefile));
  }
  string *flavors = m_indices(in);

}

#endif

public mixed *try_descend(string user_id) {
  mixed *result = AvatarMixin::try_descend(user_id);
  return result;
}

/**
 * Invoked by the login object once the avatar object is interactive and
 * has been moved to its start location.
 */
public void on_descend(string session_id, string player_id, object room, 
                       varargs mixed *args) {
  object logger = LoggerFactory->get_logger(THISO);
  AvatarMixin::on_descend(session_id);

  set_player(player_id);
  set_primary_id(query_username());
  add_secondary_id(CAP(query_username()));
  set_nickname(CAP(query_username()));
  set_homedir(HomeDir + "/" + query_username());
  set_cwd(query_homedir());
  set_short(query_nickname());
  set_long("A player object.");

  load_command_spec(UserCommandSpec);
  restore_prompt();
  // TODO restore_inventory();
  if (room) {
    if (!move_object(THISO, room)) {
      logger->warn("Unable to move player avatar to start room: %O", room);
    }
  }
  run_script(DescendScript);
}

/**
 * Return the username associated with this avatar. This name will be
 * consistent across all characters a user plays.
 *
 * @return the username
 */
public string query_username() {
  string user_id = SessionTracker->query_user(query_session());
  return UserTracker->query_username(user_id);
}

protected void set_player(string player_id) {
  player = player_id;
}

public string query_player() {
  return player;
}

protected void create() {
  HumanCode::create();
  setup();
}
