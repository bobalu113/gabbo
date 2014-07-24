/**
 * Utility library for parsing command arguments.
 * 
 * @author devo@eotl
 * @alias ArgsLib
 */

private variables private functions inherit StringsLib;

/**
 * Separate space-deliniated arguments, but keep quoted strings together.
 * 
 * @param  arg             the string containing quoted arguments
 * @param  preserve_quotes set to 1 if the quote characters should be
 *                         preserved in the resulting argument strings
 * @return                 an array of space-deliniated arguments, with the
 *                         quoted strings returned as a single argument
 */
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
