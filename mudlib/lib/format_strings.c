/**
 * Utility library for parsing format strings into runnable lambda closures.
 * 
 * @author devo
 * @alias FormatStringsLib
 */

// TODO refactor into a generic parse_format() function

/**
 * Parse a format token which takes an optional argument.
 * 
 * @param  token  the token (i.e. the 't' part of %t{%d})
 * @param  parts  an array of all the tokenized parts of the format string
 * @param  size   sizeof(parts)
 * @param  part   the part currently being parsed
 * @param  cursor the index of the part currently being parsed, passed by
 *                reference (this cursor may need to be advanced)
 * @param  def    default value of arg if no arg could be found, or 0 to 
 *                return 0 in the absense of an arg
 * @return        the argument string (i.e. the "%d" part of %t{%d})
 */
string parse_arg(int token, string *parts, int size, string part, int cursor, 
                 string def) {
  object logger = LoggerFactory->get_logger(THISO);
  string arg;
  if (sscanf(part, to_string(({ token })) + "{%s}%s", arg, part) == 2);
  else if (strlen(part) < 2) {
    part = "";
    arg = def;
  } else if (part[1] != '{') {  
    part = part[1..];
    arg = def;
  } else {
    if (strlen(part) > 2) {
      arg = part[2..];
    } else {
      arg = "";
    }

    int j = cursor + 1, pos = -1;
    string tmp;
    for (; j < size; j++) {
      if (sscanf(parts[j], "%s}%s", tmp, part) == 2) {
        pos = j;
        break;
      }
    }
    if (pos == -1) {
      logger->error("Invalid format: %%%s", part);
      return 0;
    }
    if (strlen(tmp)) { 
      tmp = "%" + tmp;
    }
    arg = sprintf("%s%s%s", arg, implode(parts[cursor+1..pos-1], "%"), tmp);
    if (arg[0] != '%') {
      arg = "%" + arg;
    }
    cursor = j;
  }
  return arg;
}

/**
 * Parse a format string into a closure that can be invoked later by the 
 * logger to format a log line. The closure will take the arguments:

   <pre>
   'verb: the verb being used to travel
   'dir:  for out msg, the exit direction. for in msg, the entrance 
          direction, or 0 if entrance could not be discerned. always 0 for
          teleport messages.
   </pre>

 * @param  format the format string
 * @return        the formatter closure, bound to THISO
 */
closure parse_mobile_format(string format) {
  string output_fmt = "";
  mixed *output_args = ({ });

  string *parts = explode(format, "%");
  int size = sizeof(parts);

  if (!size || (size == 1 && (!strlen(format) || format[0] != '%'))) {
    // static string, just use as-is
    output_args += ({ ({ #'return, format }) });
    output_fmt = "%s";
  } else {
    output_fmt += parts[0];

    for (int i = 1; i < size; i++) {
      string part = parts[i];
      switch (part[0]) {
        
        case FMT_NEWLINE: 
        output_fmt += "\n";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_PERCENT: 
        output_fmt += "%%";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_VERB:
        output_args += ({ 'verb });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_DIRECTION:
        string arg = parse_arg(FMT_OBJECT, parts, size, &part, &i, 0);
        if (!arg) {
          arg = DEFAULT_DIRECTION;
        }
        output_args += ({ #'||, 'dir, arg });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_NAME:
        string arg = parse_arg(FMT_OBJECT, parts, size, &part, &i, 0);
        if (!arg) {
          arg = DEFAULT_NAME;
        }
        output_args += ({ #'||, 
                          ({ #'call_other, 'who, "query_name" }),
                          arg
                        });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_SHORT:
        string arg = parse_arg(FMT_OBJECT, parts, size, &part, &i, 0);
        if (!arg) {
          arg = DEFAULT_SHORT;
        }
        output_args += ({ #'||, 
                          ({ #'call_other, 'who, "query_short" }),
                          arg
                        });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;
      }
    }
  }

  return lambda(({ 'who, 'verb, 'dir }), 
                ({ #'sprintf, output_fmt }) + output_args);
}


/**
 * Parse a format string into a closure that can be invoked later by the 
 * logger to format a log line. The closure will take the arguments:

   <pre>
   'category: the category of the logger
   'priority: the priority of the log event
    'message: an optional error message
     'caller: an array read from 
              <code>debug_info(DINFO_TRACE, DIT_CURRENT)</code>
              for the invoking stackframe, or 0 if no caller was found
   </pre>

 * @param  format the format string
 * @return        the formatter closure, bound to the factory
 */
closure parse_logger_format(string format) {
  string output_fmt = "";
  mixed *output_args = ({ });

  string *parts = explode(format, "%");
  int size = sizeof(parts);

  if (!size || (size == 1 && (!strlen(format) || format[0] != '%'))) {
    // static string, just use as-is
    output_args += ({ ({ #'return, format }) });
    output_fmt = "%s";
  } else {
    output_fmt += parts[0];

    for (int i = 1; i < size; i++) {
      string part = parts[i];
      switch (part[0]) {
        
        case FMT_NEWLINE: 
        output_fmt += "\n";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_PERCENT: 
        output_fmt += "%%";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_CATEGORY:
        int arg = to_int(
          parse_arg(FMT_CATEGORY, parts, size, &part, &i, "0"));
        if (arg) {
          output_args += ({ ({ #'implode, 
                          ({ #'[<.., ({ #'explode, 'category, "." }), arg }), 
                          "."
                    }) });
                     
        } else {
          output_args += ({ 'category });
        }
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_OBJECT:
        int arg = to_int(parse_arg(FMT_OBJECT, parts, size, &part, &i, "0"));
        mixed oname = ({ #'?, 
                         'caller, 
                         ({ #'[, 'caller, TRACE_PROGRAM }), 
                         "" 
                      });
        if (arg) {
          output_args += ({ ({ #'implode, 
                          ({ #'[<.., ({ #'explode, oname, "/" }), arg }), 
                          "."
                    }) });
                     
        } else {
          output_args += ({ oname });
        }
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_LOCATION:
        output_args += ({ ({ #'?,
                             'caller,
                             ({ #'sprintf, 
                                "%s->%s(%s:%d)", 
                                ({ #'[, 'caller, TRACE_OBJECT }),
                                ({ #'[, 'caller, TRACE_NAME }),
                                ({ #'[, 'caller, TRACE_PROGRAM }),
                                ({ #'[, 'caller, TRACE_LOC })
                             }),
                             ""
                       }) });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_LINE:
        output_args += ({ ({ #'?,
                             'caller,
                             ({ #'[, 'caller, TRACE_LOC }),
                             -1
                       }) });
        output_fmt += "%d";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_MSG:
        output_args += ({ 'message });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_PRIORITY:
        output_args += ({ ({ #'sprintf, "%-5s", 'priority }) });
        output_fmt += "%s";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_MILLIS:
        output_args += ({ ({ #'[, ({ #'utime }), 1 }) });
        output_fmt += "%d";
        if (strlen(part) > 1) {
          output_fmt += part[1..];
        }
        break;

        case FMT_DATE: 
        string arg = parse_arg(FMT_DATE, parts, size, &part, &i, "%F %T");
        output_args += ({ ({ #'||, 
                             ({ #'strftime, arg, ({ #'time }) }), 
                             "" 
                       }) });
        output_fmt += "%s";
        if (strlen(part)) {
          output_fmt += part;
        }
        break;
      }
    }
  }

  return lambda(({ 'category, 'priority, 'message, 'caller }), 
    ({ #'sprintf, output_fmt }) + output_args);
}

