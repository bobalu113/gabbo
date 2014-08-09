/**
 * The LoggerFactory. This service object is responsible for instantiating
 * new logger objects and configuring them from properties files located
 * throughout the filesystem. When new loggers are created, they are added to
 * a pool so that they may be reused again in a later execution without the
 * overhead of re-configuration. The factory is responsible for cleaning up
 * logger objects which are no longer in use. 
 * 
 * @alias LoggerFactory
 */
#pragma no_clone
#include <logger.h>
#include <sys/debug_info.h>
#include <sys/objectinfo.h>

private variables private functions inherit FileLib;

// FUTURE add color

default private variables;

// XXX we could probably reuse loggers between categories with some kind of
// equality test, but I don't know if it's worth it.
/** ([ str category : obj logger ]) */
mapping loggers;

/** ([ obj logger : int ref_count ]) */
mapping local_ref_counts;

/** ([ str format : cl formatter ]) */
mapping formatters;

/** A Logger instance for the factory to use */
object factory_logger;

default private functions;

public varargs object get_logger(mixed category, object rel, int reconfig);
mapping read_config(string category, string dir);
mapping read_properties(string prop_file);
string read_prop_value(mapping props, string prop, string dir, 
                       string category);
protected mixed *parse_output_prop(string val);
protected closure parse_format(string format);
string parse_arg(int token, string *parts, int size, string part, int index, 
                 string def);
string normalize_category(mixed category);
public object get_null_logger();
public int release_logger(mixed category);
int clean_up_loggers();

/**
 * Retrieve a logger instance for the given category from the pool, or create
 * a new one from configuration. A category is represented as a hierarchical
 * string of the form, "supercat.category.subcat.<...>". The category may 
 * also be specified as a filesystem path, in which case the path
 * delimiters (forward slashes) will be converted to the category delimiter
 * (periods).
 *
 * @param  category an object or string representing the category; if an 
 *                  object is specfied, it's program_name(E) will be used.
 * @param  rel      optional object to use for resolving relative paths in
 *                  configuration files; if unspecified, previous_object()
 *                  will be used.
 * @param reconfig  set this flag to 1 if you need to re-read the logger
 *                  configuration
 * @return          a logger instance
 */
public varargs object get_logger(mixed category, object rel, int reconfig) {
  // normalize some input
  category = normalize_category(category);
  if (!rel) {
    rel = previous_object();
  }
  
  // check our cache
  if (!reconfig && loggers[category]) {
    return loggers[category];
  }

  // build our configuration
  mapping config = read_config(category, load_name(rel));
  if (!member(config, "output")) {
    // no output
    return get_null_logger();
  }
  if (!member(config, "format")) {
    config["format"] = DEFAULT_FORMAT;
  }
  if (!member(config, "level")) {
    config["level"] = DEFAULT_LEVEL;
  }

  // compile formatter
  closure formatter = formatters[config["format"]];
  if (!formatter) {
    formatter = parse_format(config["format"]);
  }
  
  // configure our logger
  object logger = clone_object(Logger);
  logger->set_category(category);
  logger->set_output(config["output"]);
  logger->set_formatter(formatter);
  logger->set_level(config["level"]);
  loggers[category] = logger;
  if (!member(local_ref_counts, logger)) {
    local_ref_counts[logger] = 0;
  }
  local_ref_counts[logger]++;
  return logger;
}

/**
 * Read in logger configuration for the specified category. Starting in the
 * specified directory, this function will look for the file 
 * "etc/logger.properties", and inspect the file for any configuration 
 * properties which apply to the specified category. The result is a mapping
 * of the form:
 * <code>
 * ([ "output" : ({ ({ int type, string target }), ... }),
 *    "format" : str format_string,
 *    "level"  : str level
 * ])
 * </code>
 * 
 * @param  category the category to match configuration properties against
 * @param  dir      the starting directory from which to search for 
 *                  properties files
 * @return          the configuration mapping
 */
mapping read_config(string category, string dir) {
  mapping result = ([ ]);
  while (dir = dirname(dir)) {
    mapping props = read_properties(dir + "/" + PROP_FILE);
    if (props) {
      foreach (string prop : ALLOWED_PROPS) {
        string val = read_prop_value(props, prop, dir, category);
        if (val) {
          switch (prop) {
            case "output":
              if (!member(result, "output")) {
                mixed *output = parse_output_prop(val);
                if (output) {
                  result["output"] = output;
                }
              }
              break;
            case "format":
              if (!member(result, "format")) {
                result["format"] = val;
              }
              break;
            case "level":
              if (!member(result, "level")) {
                if (member(LEVELS, val) != -1) {
                  result["level"] = LEVELS[val];
                }
              }
              break;
          }
        }
        if (sizeof(result) == sizeof(ALLOWED_PROPS)) { break; }
      }
      if (sizeof(result) == sizeof(ALLOWED_PROPS)) { break; }
    }
  }
  return result;
}

/**
 * Parse a properties file into a mapping of property names to their values.
 * Properties are defined in a single line, of the format "name=value". Lines
 * beginning with "#" will be treated as comments.
 * 
 * @param  prop_file the path to the property file
 * @return           a mapping of property names to values, or 0 if the file
 *                   could not be read
 */
mapping read_properties(string prop_file) {
  mapping result = ([ ]);
  int count = 0;

  string body = read_file(prop_file);
  if (!body) {
    return 0;
  }
  string *lines = explode(body, "\n");
  foreach (string line : lines) {
    count++;
    if (!strlen(line)) { continue; }
    if (line[0] == '#') { continue; }

    int equals = member(line, '=');
    if (equals == -1) {
      factory_logger->warn("Malformed property on line %d of %s", 
        count, prop_file);
      continue;
    }

    string prop = line[0..(equals-1)];
    string val = line[(equals+1)..];
    result[prop] = val;
  }
  return result;
}

/**
 * Look for a property in the property mapping by name which matches a 
 * specific category, and return its value.
 * 
 * @param  props    the property map
 * @param  prop     the name of the property to find
 * @param  path     categories in the property file will be resolved relative 
 *                  to this path (delimited by periods or forward slashes)
 * @param  category the category which a property must match to be returned
 * @return          the property value for the specified property name and
 *                  category, or 0 if no matching property could be found
 */
string read_prop_value(mapping props, string prop, string path, 
                       string category) {
  path = implode(explode(path, "/"), ".");
  if (category[0..(strlen(path)-1)] != path) {
    return 0;
  }
  string rel_category = category[strlen(path)..];
  while (1) {
    string prop_name = sprintf("%s%s.%s", PROP_PREFIX, rel_category, prop);
    if (member(props, prop_name)) {
      return props[prop_name];
    }
    int pos = rmember(rel_category, '.');
    if (pos < 0) {
      break;
    }
    rel_category = rel_category[0..(pos-1)];
  }
  return 0;  
}

/**
 * Translate the property value of an output property to something more 
 * structured than a string. The result will be of the form:
 * 
 * <code>({ ({ int type, string target }), ... })</code>
 * 
 * where type is one of 'c' or 'f' and target is an object spec or a file 
 * path, for console output or file output, respectively.
 * 
 * @param  val the value of the output property
 * @return     an array of output targets
 */
protected mixed *parse_output_prop(string val) {
  mixed *result = ({ });
  string *targets = explode(val, ",");
  foreach (string spec : targets) {
    if (spec[1] != ':') {
      factory_logger->warn("Malformed output spec: %s", spec);
      continue;
    }
    int type = spec[0];
    string target = spec[2..];
    result += ({ ({ type, target }) });
  }
  return result;
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
protected closure parse_format(string format) {
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

/**
 * Parse a format token which takes an optional argument.
 * @param  token the token (i.e. the 't' part of %t{%d})
 * @param  parts an array of all the tokenized parts of the format string
 * @param  size  sizeof(parts)
 * @param  part  the part currently being parsed
 * @param  index the index of the part currently being parsed, passed by
 *               reference (the iterator may need to be advanced)
 * @param  def   default value of arg if no arg could be found
 * @return       the argument string (i.e. the "%d" part of %t{%d})
 */
string parse_arg(int token, string *parts, int size, string part, int index, 
                 string def) {
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

    int j = index + 1, pos = -1;
    string tmp;
    for (; j < size; j++) {
      if (sscanf(parts[j], "%s}%s", tmp, part) == 2) {
        pos = j;
        break;
      }
    }
    if (pos == -1) {
      factory_logger->error("Invalid logger format: %%%s", part);
      return 0;
    }
    if (strlen(tmp)) {
      tmp = "%" + tmp;
    }
    arg = sprintf("%s%s%s", arg, implode(parts[index+1..pos-1], "%"), tmp);
    if (arg[0] != '%') {
      arg = "%" + arg;
    }
    index = j;
  }
  return arg;
}

/**
 * Derive the cannonical category name from object or filesystem path input.
 * @param  category an object, filesystem path, or category name
 * @return          the cannonical category 
 */
string normalize_category(mixed category) {
  if (objectp(category)) {
    category = program_name(category);
  } else if (!stringp(category)) {
    raise_error("Bad argument 1 to normalize_category()");
  }
  if (category[<2..<1] == ".c") {
    category = category[0..<3];
  }
  category = regreplace(category, "/", ".", 1);
  return category;
}

/**
 * Return a new no-op logger.
 * @return the logger instance
 */
public object get_null_logger() {
  object logger = clone_object(Logger);
  logger->set_category("");
  logger->set_output(({ }));
  logger->set_formatter("");
  logger->set_level(LVL_OFF);
  return logger;
}

/**
 * Releases a logger object from the logger pool, thereby removing any 
 * references to it held by the factory. If there are no other references,
 * the logger will also be destructed.
 * 
 * @param  category an object or string representing the category; if an 
 *                  object is specfied, it's program_name(E) will be used.
 * @return          1 if a logger was released, 0 if no logger was found
 */
public int release_logger(mixed category) {
  category = normalize_category(category);
  object logger = loggers[category];
  if (logger) {
    m_delete(loggers, category);
    local_ref_counts[logger]--;
    int ref_count = (int) object_info(logger, OINFO_BASIC, OIB_REF);
    int local_ref_count = local_ref_counts[logger];
    if ((ref_count - local_ref_count) <= STANDING_REF_COUNT) {
      m_delete(local_ref_counts, logger);
      destruct(logger);
    }
    return 1;
  }
  return 0;
}

/**
 * Clean up stale loggers, called once per reset. A logger is considered 
 * stale if it hasn't been referenced in 5 minutes, and is referenced by 
 * nothing except the LoggerFactory.
 * 
 * @return the number of logging categories released (may be more than the
 *         number loggers destructed if loggers are shared between categories)
 */
int clean_up_loggers() {
  int result = 0;
  foreach (string category, object logger : loggers) {
    if (!logger) { continue; } 

    int ref_time = (int) object_info(logger, OINFO_BASIC, OIB_TIME_OF_REF);
    if ((time() - ref_time) >= LOGGER_STALE_TIME) {
      result += release_logger(category);
    }
  }
  return result;
}

/**
 * Initialize logger and formatter maps.
 * @return number of seconds until first reset
 */
public int create() {
  seteuid(getuid());
  loggers = ([ ]);
  formatters = ([ ]);
  local_ref_counts = ([ ]);
  factory_logger = clone_object(Logger);
  factory_logger->set_category(normalize_category(THISO));
  factory_logger->set_output(parse_output_prop("f:/log/logger_factory.log"));
  factory_logger->set_formatter(parse_format(DEFAULT_FORMAT));
  factory_logger->set_level(LVL_WARN);
  return FACTORY_RESET_TIME;
}

/**
 * Clean up stale loggers.
 * @return number of seconds until next reset
 */
public int reset() {
  clean_up_loggers();
  return FACTORY_RESET_TIME;
}

