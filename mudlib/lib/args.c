inherit StringsLib;

string *explode_args(string arg, int keepquotes) {
  string *args = ({ });  
  if (!arg) { return args; }
  
  int pos = 0;
  int len = strlen(arg);
  while (pos < len) {
    pos = find_nonws(arg, pos);
    int newpos = match_quote(arg, pos);
    if (keepquotes) {
      args += ({ arg[pos..newpos] });
    } else {
      args += ({ unescape(unquote(arg[pos..newpos])) });
    }
    pos = newpos + 1;
  }
  return args;
}
