inherit CommandCode;

int do_command(string arg) {
  // TODO add path expansion
  // TODO add -f option to force
  // XXX messaging?
  
  object target = FINDO(arg);
  if (!target) {
    if (!file_exists(arg)) {
      printf("%s: %s: No such object.\n", query_verb(), arg);
      return 1;
    }
  } else {
    destruct(target);
    if (target) {
      printf("%s: %s: Unable to destruct.\n", query_verb(), arg);
      return 1;
    }
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
