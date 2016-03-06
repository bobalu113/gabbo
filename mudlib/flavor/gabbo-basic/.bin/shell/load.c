inherit CommandCode;

private variables private functions inherit ArgsLib;
private variables private functions inherit GetoptsLib;
private variables private functions inherit FileLib;
private variables private functions inherit ObjectLib;

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
  string out = "";
  foreach (mixed *f : files) {
    string file = f[0];
    if (FINDO(file)) {
      out += sprintf("%s: %s: Already loaded. Destruct or use the reload "
                     "command.\n",
                     query_verb(), file);
      continue;
    }

    string err = catch (load_object(file); publish);
    if (err) {
      out += sprintf("%s: %s: Caught error %s\n", 
                     query_verb(), file, err); 
      mixed *last_err = get_error_file(MasterObject->get_wiz_name(file));
      if (last_err) {
        out += sprintf("%s line %d: %s\n", 
                       last_err[0], last_err[1], last_err[2]);
      }
      continue;
    } else {
      count++;
      if (member(arg, 'v')) {
        out += sprintf("%s: %s: loaded\n", query_verb(), file);
      }
    }
  }
  
  out += sprintf("%s: %s: %d object%s loaded.\n", 
                 query_verb(), implode(args[0], " "), 
                 count, (count != 1 ? "s" : ""));
  tell_player(THISP, out);
  return 1;
}
