inherit CommandCode;

mixed *query_actions() {
  return ({ ({ "chdir", 0 }), ({ "cd", 0 }) });
}

int do_command(string arg) {
  if (!arg || !strlen(arg)) {
    arg = HomeDir "/" + PID;
  }

  // TODO add path expansion
  if (!file_exists(arg)) {
    printf("%s: %s: no such directory or permission denied.\n", 
      query_verb(), arg);
    return 1;
  }
  THISP->set_cwd(arg);
  return 1;
}
