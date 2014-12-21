inherit CommandCode;

private variables private functions inherit ArgsLib;
private variables private functions inherit GetoptsLib;
private variables private functions inherit FileLib;

int do_command(string arg) {
  // TODO add -f option to force
  // XXX messaging?
  mixed *args = getopts(explode_args(arg), "v");

  if (!sizeof(args[0])) {
    notify_fail(sprintf("Usage: %s file\n", query_verb()));
    return 0;
  }

  mixed *files = ({ });
  foreach (string a : args[0]) {
    files += expand_pattern(a, THISP);
  }

  if (!sizeof(files)) {
    printf("%s: %s: No such file.\n", query_verb(), implode(args[0], " "));
    return 1;
  }

  int count = 0;
  foreach (mixed *file : files) {
    if (FINDO(file[0])) {
      printf("%s: %s: Already loaded. Destruct or use the reload command.\n",
             query_verb(), file[0]);
      return 1;
    }

    string err = catch (load_object(file[0]); publish);
    if (err) {
      printf("%s: %s: Caught error %s\n", query_verb(), file[0], err); 
      mixed *last_err = get_error_file(MasterObject->get_wiz_name(args[0]));
      if (last_err) {
        printf("%s line %d: %s\n", last_err[0], last_err[1], last_err[2]);
      }
      return 1;
    } else {
      count++;
      if (member(arg, 'v')) {
        printf("%s: %s: loaded\n", query_verb(), file[0]);
      }
    }
  }
  
  printf("%s: %s: %d object%s loaded.\n", 
         query_verb(), implode(args[0], " "), 
         count, (count != 1 ? "s" : ""));
  return 1;
}
