/**
 * Utility library for parsing format strings into runnable lambda closures.
 *
 * @author devo
 * @alias FormatStringsLib
 */

#include <sys/debug_info.h>

#define DEFAULT_ARG  0
#define SPRINTF_FMT  1
#define FMT_LAMBDA   2

#define ARG_OPEN     "{["
#define ARG_CLOSE    "}]"

private variables private functions inherit ArgsLib;
private variables private functions inherit StringsLib;
private variables private functions inherit ClosureLib;

/**
 * Compile a format string into a formatter closure with the given format
 * specifiers and args. Format strings may contain tokens of the format "%c"
 * where 'c' is some alphabetic character. They may also take arguments in
 * the format of "%c{arg}". For every format specifier found, the returned
 * closure will run the closure found for the character in infomap (the
 * character is the key) and replace the format specifier with the result of
 * the evaluated closure. Closures may reference the optional arg value with
 * the symbol 'arg. They may also reference any arguments passed to the
 * closure at runtime with the symbols defined in the args parameter. (Values
 * for these arguments must be supplied by the caller at runtime!) References
 * to the closure arguments in infomap must be double quoted even though
 * they are single quoted in the args parameter.
 *
 * Invoking the result of this function will return the format string with
 * all constituent format specifiers replaced by the result of their closure
 * calls. Example:
 *
 * closure formatter =
 *   parse_format_string(
 *     "name: %N",        // format string
 *     ([ 'N' : ({ 0,     // default value for 'arg
 *                 "%s",  // sprintf() format string of result
 *                 ({     // array of closures to call
 *                        // results will be passed as args to sprintf()
 *                    ({ #'call_other, ''who, "query_name" })
 *                 })
 *              })
 *     ]),
 *     ({ 'who })         // formatter will take one arg named 'who
 *   );
 * // this should write "devo"
 * write(funcall(formatter, FINDP("devo")));
 *
 * @param  format  the format string to parse
 * @param  infomap a map of format specifiers to the info array of how to
 *                 replace the specifier
 * @param  args    optional args that will be passed to the formatter
 *                 (single quoted)
 * @return         a formatter closure that takes specified args and will
 *                 produce formatted strings according to infomap
 */
closure parse_format(string format, mapping infomap, symbol *args) {
  object logger = LoggerFactory->get_logger(THISO);
  string output_fmt = "";
  mixed *output_args = ({ });

  string *parts = explode_nested(format, "%", ARG_OPEN, ARG_CLOSE);
  int size = sizeof(parts);

  output_fmt += parts[0]; // first part is always leading text (could be "")
  for (int i = 1; i < size; i++) {
    string part = unescape(parts[i]);
    if (!strlen(part)) { continue; } // most likely %% in the string
    int spec = part[0];

    if (!member(infomap, spec)) {
      logger->warn("Unknown format specifier: %c", spec);
      continue;
    }
    mixed *info = infomap[spec];

    string arg = info[DEFAULT_ARG];
    string extra = part[1..];
    if (strlen(part) > 1) {
      int pos = find_close_char(part, 1, ARG_OPEN, ARG_CLOSE);
      if (pos > 0) {
        arg = part[2..(pos - 1)];
        extra = part[(pos + 1)..];
      } else if (pos < 0) {
        logger->warn("Unmatched '%c': %%%s", part[1], part);
      }
    }

    output_fmt = sprintf("%s%s%s", output_fmt, info[SPRINTF_FMT], extra);
    output_args += funcall(
      reconstruct_lambda(({ 'arg }), info[FMT_LAMBDA]),
      arg);
  }

  return lambda(args, ({ #'sprintf, output_fmt }) + output_args);
}
