inherit CommandCode;

int do_command(string arg) {
  // FUTURE figure out what to do when called argless

  if (!arg || !strlen(arg)) {
    notify_fail(sprintf("Usage: %s file\n", query_verb()));
    return 0;
  }

  // TODO add path expansion

  ed(arg);
  return 1;
}
