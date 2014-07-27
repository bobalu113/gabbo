/**
 * A logger object which is capable of writing to log files, or the console
 * of users. This object should not be cloned directly, but rather be 
 * accessed through LoggerFactory->get_logger().
 * 
 * @alias Logger
 */
#include <logger.h>
#include <sys/debug_info.h>

default private variables;

string category;

mixed *output;

closure formatter;

string level;

default public functions;

int is_enabled(string priority);
void log(string priority, string message, varargs string *args);
private void do_output(string msg);
private mixed *find_caller();
private object *expand_objects(string ospec);

/**
 * Get the logger's category.
 * @return the period-delimited full category path of the logger
 */
string query_category() {
  return category;
}

/**
 * Get the output specs.
 * @return an array of output targets, of the form 
 * 
 *         <code>({ int spec : string target })</code>
 *   
 *         where type is one of 'c' or 'f' and target is an object spec or a 
 *         file path, for console output or file output, respectively.
 */
mixed *query_output() {
  return output;
}

/**
 * Get the closure which will be used to format logging events for output.
 * The closure will take the arguments:
 * 
 * <pre>
   'category: the category of the logger
   'priority: the priority of the log event
    'message: an optional error message
     'caller: an array read from 
              <code>debug_info(DINFO_TRACE, DIT_CURRENT)</code>
              for the invoking stackframe, or 0 if no caller was found
   </pre>
 *  
 * @return the formatter closure, bound to LoggerFactory
 */
closure query_formatter() {
  return formatter;
}

/**
 * Get the minimum level for which messages will be output.
 * @return the minimum log level
 */
string query_level() {
  return level;
}

/**
 * Set the logger category.
 * @param  str the category to set
 * @return     1 for success, 0 for failure
 */
int set_category(string str) { 
  category = str;
  return 1;
}

/**
 * Set the logger output target list.
 * @param  arr the output targets
 * @return     1 for success, 0 for failure
 */
int set_output(mixed *arr) {
  output = arr;
  return 1;
}

/**
 * Set the formatter.
 * @param  cl the formatter to set
 * @return    1 for success, 0 for failure
 */
int set_formatter(closure cl) {
  formatter = cl;
  return 1;
}

/**
 * Set the minimum log level.
 * @param  str the log level to set
 * @return     1 for success, 0 for failure
 */
int set_level(string str) {
  level = str;
  return 1;
}

/**
 * Test whether logging is enabled for a specified priority.
 * 
 * @param  priority the priority of the logging event
 * @return          1 if logging is enabled, otherwise 0
 */
int is_enabled(string priority) {
  return level >= LEVELS[priority]; 
}

/**
 * Test whether logging is enabled for FATAL events.
 * 
 * @return          1 if logging is enabled, otherwise 0
 */
int is_fatal_enabled() {
  return is_enabled(LVL_FATAL);
}

/**
 * Test whether logging is enabled for ERROR events.
 * 
 * @return          1 if logging is enabled, otherwise 0
 */
int is_error_enabled() {
  return is_enabled(LVL_ERROR);
}

/**
 * Test whether logging is enabled for WARN events.
 * 
 * @return          1 if logging is enabled, otherwise 0
 */
int is_warn_enabled() {
  return is_enabled(LVL_WARN);
}

/**
 * Test whether logging is enabled for INFO events.
 * 
 * @return          1 if logging is enabled, otherwise 0
 */
int is_info_enabled() {
  return is_enabled(LVL_INFO);
}

/**
 * Test whether logging is enabled for DEBUG events.
 * 
 * @return          1 if logging is enabled, otherwise 0
 */
int is_debug_enabled() {
  return is_enabled(LVL_DEBUG);
}

/**
 * Test whether logging is enabled for TRACE events.
 * 
 * @return          1 if logging is enabled, otherwise 0
 */
int is_trace_enabled() {
  return is_enabled(LVL_TRACE);
}

/**
 * Record a log message with the FATAL priority.
 * 
 * @param msg_fmt the log message format string 
 * @param args    the values to be passed to sprintf() as args
 */
void fatal(string msg_fmt, varargs string *args) {
  apply(#'log, LVL_FATAL, msg_fmt, args);
}

/**
 * Record a log message with the ERROR priority.
 * 
 * @param msg_fmt the log message format string 
 * @param args    the values to be passed to sprintf() as args
 */
void error(string msg_fmt, varargs string *args) {
  apply(#'log, LVL_ERROR, msg_fmt, args);
}

/**
 * Record a log message with the WARN priority.
 * 
 * @param msg_fmt the log message format string 
 * @param args    the values to be passed to sprintf() as args
 */
void warn(string msg_fmt, varargs string *args) {
  apply(#'log, LVL_WARN, msg_fmt, args);
}

/**
 * Record a log message with the INFO priority.
 * 
 * @param msg_fmt the log message format string 
 * @param args    the values to be passed to sprintf() as args
 */
void info(string msg_fmt, varargs string *args) {
  apply(#'log, LVL_INFO, msg_fmt, args);
}

/**
 * Record a log message with the DEBUG priority.
 * 
 * @param msg_fmt the log message format string 
 * @param args    the values to be passed to sprintf() as args
 */
void debug(string msg_fmt, varargs string *args) {
  apply(#'log, LVL_DEBUG, msg_fmt, args);
}

/**
 * Record a log message with the TRACE priority.
 * 
 * @param msg_fmt the log message format string 
 * @param args    the values to be passed to sprintf() as args
 */
void trace(string msg_fmt, varargs string *args) {
  apply(#'log, LVL_TRACE, msg_fmt, args);
}

/**
 * Record a log message with the specified priority.
 *
 * @param priority the priority of the log message
 * @param msg_fmt  the log message format string 
 * @param args     the values to be passed to sprintf() as args
 */
void log(string priority, string msg_fmt, varargs string *args) {
  if (!is_enabled(priority)) {
    return;
  }

  string msg = apply(#'sprintf, ({ msg_fmt }) + args);
  mixed *caller = find_caller();
  msg = funcall(formatter, category, priority, msg, caller);
  do_output(msg);
}

/**
 * Output the formatted log message to all the configured places.
 * @param msg the formatted log message
 */
private void do_output(string msg) {
  foreach (mixed *target : output) {
    switch (target[0]) {
      case OUT_CONSOLE:
      foreach (object ob : expand_objects(target[1])) {
        tell_object(ob, msg + "\n");
      }
      break;

      case OUT_FILE:
      write_file(target[1], msg + "\n");
      break;
    }
  }
}

/**
 * Find the last external call in the call stack and return it.
 * @return the info for the last external stack frame from 
 * 
 *         <code>debug_info(DINFO_TRACE, DIT_CURRENT)</code>
 *         
 *         or 0 if no external caller was found.
 */
private mixed *find_caller() {
  mixed *stack = debug_info(DINFO_TRACE, DIT_CURRENT);
  int i = sizeof(stack) - 1;
  for (; i >= 1; i--) {
    if (strlen(stack[i][2]) && (stack[i][2] == __FILE__)) {
      continue;
    }
    break;
  }
  if (i >= 1) {
    return stack[i];
  }
  return 0;
}

/**
 * Resolve an ospec. This is just a stub.
 * @param  ospec the ospec
 * @return       the matching objects
 */
private object *expand_objects(string ospec) {
  if (ospec == "thisp") {
    return ({ THISP });
  } else {
    return ({ });
  }
}