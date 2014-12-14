/**
 * A standard avatar for player characters.
 * 
 * @author devo@eotl
 * @alias Avatar
 */

inherit LivingCode;

inherit NameMixin;
inherit VisibleMixin;
inherit ShellMixin;
inherit CommandGiverMixin;

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
  LivingCode::create();
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

  set_username(username);
  set_primary_id(username);
  add_secondary_id(CAP(username));
  set_nickname(CAP(username));
  set_cwd(HomeDir + "/" + username);
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
    BinDir "/put"
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
      ({ #'call_other, THISO, "query_cwd" }), 
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
