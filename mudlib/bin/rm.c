inherit CommandCode;

private variables private functions inherit FileLib;

int do_command(string arg) {
  if (!arg || !strlen(arg)) {
    notify_fail(sprintf("Usage: %s file\n", query_verb()));
    return 0;
  }

  // TODO add path expansion
  if (!file_exists(arg)) {
    printf("%s: %s: no such file or permission denied.\n", query_verb(), arg);
    return 1;
  }

  if (is_directory(arg)) {
    printf("%s: %s: use rmdir to remove directory.\n", query_verb(), arg);
    return 1;
  }

  if (!rm(arg)) {
    printf("%s: %s: Unable to remove file.\n", query_verb(), arg);
    return 1;
  }

  return 1;
}
