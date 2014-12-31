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
