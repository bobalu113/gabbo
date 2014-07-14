inherit StringsLib;

varargs string *explode_args(string arg, int preserve_quotes) {
  string *args = ({ });  
  if (!arg) { return args; }
  
  int pos = 0;
  int len = strlen(arg);
  while (pos < len) {
    pos = find_nonws(arg, pos);
    int newpos = match_quote(arg, pos);
    if (preserve_quotes) {
      args += ({ arg[pos..newpos] });
    } else {
      args += ({ unescape(unquote(arg[pos..newpos])) });
    }
    pos = newpos + 1;
  }
  return args;
}
