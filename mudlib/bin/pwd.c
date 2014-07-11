inherit CommandCode;

int do_command(string arg) {
  if (stringp(arg) && strlen(arg) > 0) {
    notify_fail(sprintf("%s: too many arguments\n", query_verb()));
    return 0;
  }
  printf("%s\n", THISP->query_cwd());
  return 1;
}
