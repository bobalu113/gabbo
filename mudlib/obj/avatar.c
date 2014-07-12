inherit FileLib;

/* names and ids and stuff */

string name;
string id;

string query_name() {
  return name;
}

string query_id() {
  return id;
}

string *query_secondary_ids() {
  return ({ "player" });
}

string *query_ids() {
  return ({ query_id() }) + query_secondary_ids();
}

/* command routing */

// ([ verb : command_file ])
mapping verbs;

// to keep track if a command has been reloaded since init
// ([ command_object ])
nosave mapping commands;

void initialize_actions() {
  verbs = ([ ]);
  commands = ([ ]);

  // TODO make this configuration-driven
  string *command_files = ({ 
    BinDir "/pwd",
    BinDir "/chdir",
    BinDir "/mkdir",
    BinDir "/rmdir",
    BinDir "/ls", 
    BinDir "/grep", 
    BinDir "/rm", 
    BinDir "/touch",
    BinDir "/ed", 
    BinDir "/more",
    BinDir "/cc", 
    BinDir "/call", 
    BinDir "/tail", 
    BinDir "/load", 
    BinDir "/reload", 
    BinDir "/clone",
    BinDir "/dest", 
    BinDir "/man"
  });

  foreach (string command : command_files) {
    object cmd_ob;
    string err = catch(cmd_ob = load_object(command));
    if (err) {
      // TODO log error
      printf("Caught error loading command %s: %s\n", command, err);
      continue;
    }
    mixed *actions = cmd_ob->query_actions();
    foreach (mixed *action : actions) {
      string verb = action[0];
      int flag = action[1];
      verbs[verb] = command;
      commands += ([ cmd_ob ]);
      add_action("do_command", verb, flag);
    }
  }
}

static int do_command(string arg) {
  string verb = query_verb(1);
  if (!member(verbs, verb)) {
    // TODO log warning
    return 0;
  }
  string command = verbs[verb];
  object cmd_ob = FINDO(command);
  if (!cmd_ob || !member(commands, cmd_ob)) {
    // FUTURE prompt user whether or not to re-init
    string err = catch(cmd_ob = load_object(command));
    // TODO consolidate command loading logic (see above)
    if (err) {
      // TODO log error
      printf("Caught error loading command %s: %s\n", command, err);
      return 0;
    } else {
      commands += ([ cmd_ob ]);
    }
  }

  return cmd_ob->do_command(arg);
}


/* file management */

string cwd;

int set_cwd(string dir) {
  // FUTURE add security
  if (!file_exists(dir)) {
    return 0;
  }
  cwd = dir;
  return 1;
}

string query_cwd() {
  return cwd;
}


/* initialization */

int setup(string username) {
  id = username;
  name = CAP(username);
  cwd = HomeDir + "/" + username;
  return 0;
}

void enter_game() {
  enable_commands();
  initialize_actions();
  set_prompt(lambda(0, 
    ({ #'+, 
      ({ #'call_other, THISO, "query_cwd" }), 
      ">" 
    })
  ), THISO);
}
