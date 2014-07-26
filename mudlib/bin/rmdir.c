inherit CommandCode;

private variables private functions inherit FileLib;

int do_command(string arg) {
  if (!arg || !strlen(arg)) {
    notify_fail(sprintf("Usage: %s dir\n", query_verb()));
    return 0;
  }

  // TODO add path expansion
  if (!file_exists(arg)) {
    printf("%s: %s: no such directory or permission denied.\n", 
      query_verb(), arg);
    return 1;
  }

  if (!is_directory(arg)) {
    printf("%s: %s: not a directory.\n", query_verb(), arg);
    return 1;
  }

  if (!rmdir(arg)) {
    printf("%s: %s: Unable to remove directory.\n", query_verb(), arg);
    return 1;
  }

  return 1;
}
