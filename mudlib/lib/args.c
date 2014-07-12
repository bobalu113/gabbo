inherit StringsLib;

string *explode_args(string arg) {
  string *args = ({ });  
  if (!arg) { return args; }
  
  int pos = 0;
  int len = strlen(arg);
  while(pos < len) {
    pos = find_nonws(arg, pos);
    int newpos = match_quote(arg, pos);
    args += ({ unescape(unquote(arg[pos..newpos])) });
    pos = newpos + 1;
  }
  return args;
}

