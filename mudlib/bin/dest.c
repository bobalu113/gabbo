inherit CommandCode;

int do_command(string arg) {
  // TODO add object expansion
  // TODO add -f option to force
  // XXX messaging?

  object ob = FINDO(arg);
  if (!ob) {
    printf("%s: %s: No such object.\n", query_verb(), arg);
    return 1;
  }

  string err = catch (destruct(ob); publish);
  if (err) {
    printf("%s: %s: Caught error %s\n", query_verb(), arg, err); 
    return 1;
  }

  printf("%s: %s: 1 object destructed.\n", query_verb(), arg);
  return 1;
}
