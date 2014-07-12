inherit CommandCode;

int do_command(string arg) {
  // TODO add path expansion
  // TODO add -f option to force
  // XXX messaging?

  if (FINDO(arg)) {
    printf("%s: %s: Already loaded. Destruct or use the reload command.\n",
      query_verb(), arg);
    return 1;
  }

  if (!file_exists(arg)) {
    printf("%s: %s: No such file.\n", query_verb(), arg);
    return 1;
  }

  string err = catch(load_object(arg); publish);
  if (err) {
    printf("%s: %s: Caught error %s\n", query_verb(), arg, err); 
    mixed *last_err = get_error_file(MASTER_OBJECT->get_wiz_name(arg));
    if (last_err) {
      printf("%s line %d: %s\n", last_err[0], last_err[1], last_err[2]);
    }
    return 1;
  }

  printf("%s: %s: 1 object loaded.\n", query_verb(), arg);
  return 1;
}
