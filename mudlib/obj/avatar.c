/**
 * A standard avatar for player characters.
 *
 * @author devo@eotl
 * @alias Avatar
 */

#include <sys/functionlist.h>

inherit OrganismCode;

inherit NameMixin;
inherit VisibleMixin;
inherit ShellMixin;
inherit CommandGiverMixin;
inherit MobileMixin;

/* usernames */

private string username;

/**
 * Return the username associated with this avatar. This name will be
 * consistent across all characters a user plays.
 *
 * @return the username
 */
public string query_username() {
  return username;
}

/**
 * Set the username for this avatar.
 *
 * @param str the username to set
 * @return 0 for failure, 1 for success
 */
protected int set_username(string str) {
  username = str;
  return 1;
}

/* telnet */

/**
 * Returns the user's screen width. Currently hard-coded to 80 characters.
 * @return the user's screen width
 */
public int query_screen_width() {
  return 80;
}

/**
 * Returns the user's screen length. Currently hard-coded to 25 lines.
 * @return the user's screen length
 */
public int query_screen_length() {
  return 25;
}

/* initialization */

public void create() {
  OrganismCode::create();
}

/**
 * Invoked by the login object to set up a newly spawned avatar. At the time
 * this lfun is called, interactivity has not yet been transfered to the
 * avatar. It also has not yet been moved to the player's starting location.
 * @param  username the username to which this avatar belongs
 */
public void setup(string username) {
  // TODO previous_object check
  setup_name();
  setup_visible();
  setup_shell();
  setup_command_giver();
  setup_mobile();

  set_username(username);
  set_primary_id(username);
  add_secondary_id(CAP(username));
  set_nickname(CAP(username));
  set_homedir(HomeDir + "/" + username);
  set_cwd(query_homedir());
  set_short(query_nickname());
  set_long("A player object.");
  return;
}

/**
 * Temporary implementation to initialize a static list of commands. Will be
 * replaced with configuration-driven logic instead.
 */
protected void setup_command_giver() {
  CommandGiverMixin::setup_command_giver();

  // TODO make this configuration-driven
  string *command_files = ({
    BinDir "/pwd",
    BinDir "/chdir",
    BinDir "/mkdir",
    BinDir "/rmdir",
    BinDir "/ls",
    BinDir "/cp",
    BinDir "/mv",
    BinDir "/rm",
    BinDir "/touch",
    BinDir "/ed",
    BinDir "/more",
    BinDir "/tail",
    BinDir "/grep",
    BinDir "/cc",
    BinDir "/call",
    BinDir "/load",
    BinDir "/reload",
    BinDir "/clone",
    BinDir "/dest",
    BinDir "/man",
    BinDir "/look",
    BinDir "/get",
    BinDir "/drop",
    BinDir "/put",
    BinDir "/goto",
    BinDir "/trans",
    BinDir "/walk",
    BinDir "/follow",
    BinDir "/trace",
    BinDir "/logger",
    BinDir "/qvars",
    BinDir "/which",
    BinDir "/ghi"
  });

  foreach (string command : command_files) {
    object cmd_ob = load_command(command);
    if (!cmd_ob) {
      continue;
    }
    mixed *actions = cmd_ob->query_actions();
    foreach (mixed *action : actions) {
      string verb = action[0];
      int flag = action[1];
      add_command(command, verb, flag);
    }
  }
}

/**
 * Implementation of the modify_command hook. Every command executed by the
 * player will have a chance to be modified before being parsed by this
 * function.
 *
 * @param  cmd the command string being executed
 * @return     the new command string to execute
 */
mixed modify_command(string cmd) {
  // TODO add support for default exit verb setting
  if (ENV(THISO)->query_exit(cmd)) {
    return "walk " + cmd;
  }
  return 0;
}

/**
 * Add actions for all a player's currently configured commands.
 */
private void initialize_actions() {
  foreach (string verb, string command, int flag : query_verbs()) {
    add_action("do_command", verb, flag);
  }
}

/**
 * Invoked by the login object once the avatar object is interactive and
 * has been moved to its start location.
 */
void enter_game() {
  enable_commands();
  initialize_actions();
  set_prompt(lambda(0,
    ({ #'+,
      ({ #'call_other, THISO, "query_context" }),
      "> "
    })
  ), THISO);
}

/**
 * Returns true to designate that this object represents a player character.
 *
 * @return 1
 */
nomask int is_avatar() {
  return 1;
}

/**
 * Return a zero-width mapping of the capabilities this program provides.
 *
 * @return a zero-width mapping of capabilities
 */
public mapping query_capabilities() {
  return   OrganismCode::query_capabilities()
            + NameMixin::query_capabilities()
         + VisibleMixin::query_capabilities()
           + ShellMixin::query_capabilities()
    + CommandGiverMixin::query_capabilities()
          + MobileMixin::query_capabilities();
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