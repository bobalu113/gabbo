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
    return 1;
  }

  printf("%s: %s: 1 object loaded.\n", query_verb(), arg);
  return 1;
}
