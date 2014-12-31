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

private variables private functions inherit ArrayLib;

/**
 * Parse a format token which takes an optional argument.
 *
 * @param  parts  an array of all the tokenized parts of the format string
 * @param  size   sizeof(parts)
 * @param  part   the part currently being parsed, minus the leading format
 *                token, passed by reference; after this function finishes,
 *                any args found in part will be parsed out and its value
 *                will be any remaining content found after arg
 * @param  cursor the index of the part currently being parsed, passed by
 *                reference (this cursor may need to be advanced if arg spans
 *                across parts)
 * @param  def    default value of arg if no arg could be found, or 0 to
 *                return 0 in the absense of an arg
 * @return        the argument string (i.e. the "%d" part of %t{%d})
 */
string parse_arg(string *parts, int size, string part, int cursor,
                 string def) {
  object logger = LoggerFactory->get_logger(THISO);
  string arg;
  if (sscanf(part, "{%s}%s", arg, part) == 2) {
    // found entire arg in this part, parse it out
  } else if (!strlen(part)) {
    // no arg and no more chars, assign default arg
    arg = def;
  } else if (part[0] != '{') {
    // no arg but extra chars, assign default and normalize part
    arg = def;
  } else {
    if (strlen(part) > 1) {
      // arg found, parse out all remaining content from the current part
      arg = part[1..];
    } else {
      // no more content left in this part, initialize arg with empty string
      arg = "";
    }

    // now go looking for close brace in parts that follow
    int j = cursor + 1, pos = -1;
    string tmp;
    for (; j < size; j++) {
      if (sscanf(parts[j], "%s}%s", tmp, part) == 2) {
        pos = j;
        break;
      }
    }
    if (pos == -1) {
      // no close brace found
      logger->error("Invalid format: %%%s", part);
      return 0;
    }
    // concatenate arg-so-far, all the arg-containing parts, and whatever was
    // found inside the closing curly brace, restoring all the format tokens
    arg = sprintf("%s%s%%%s", arg, implode(parts[cursor+1..pos-1], "%"), tmp);
    /* I dunno why this is here
    if (arg[0] != '%') {
      arg = "%" + arg;
    }
    */
    // move cursor forward to next part
    cursor = j;
  }
  return arg;
}

closure parse_format(string format, mapping infomap, symbol *args) {
  object logger = LoggerFactory->get_logger(THISO);
  string output_fmt = "";
  mixed *output_args = ({ });

  string *parts = explode(format, "%");
  int size = sizeof(parts);

  if (!size || (size == 1 && (!strlen(format) || format[0] != '%'))) {
    // static string, just use as-is
    output_fmt = format;
  } else {
    output_fmt += parts[0];

    for (int i = 1; i < size; i++) {
      string part = parts[i][1..];
      if (!member(infomap, parts[i][0])) {
        logger->warn("Unknown format specifier: %c", parts[i][0]);
        continue;
      }
      mixed *info = infomap[parts[i][0]];
      output_fmt = sprintf("%s%s%s", output_fmt, info[SPRINTF_FMT], part);
      string arg = parse_arg(parts, size, &part, &i, info[DEFAULT_ARG]);
      output_args += funcall(
        reconstruct_lambda(({ 'arg }), info[FMT_LAMBDA]),
        arg);
    }
  }

  return lambda(args, ({ #'sprintf, output_fmt }) + output_args);
}
