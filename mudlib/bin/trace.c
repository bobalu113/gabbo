inherit CommandCode;

#include <sys/trace.h>

private variables private functions inherit ArgsLib;
private variables private functions inherit GetoptsLib;

int do_command(string arg) {
  mixed *args;
  args = getopts(explode_args(arg), "loraxhdpv");
  int flags = 0;
  if (member(args[1], 'l')) { flags |= TRACE_CALL; }
  if (member(args[1], 'o')) { flags |= TRACE_CALL_OTHER; }
  if (member(args[1], 'r')) { flags |= TRACE_RETURN; }
  if (member(args[1], 'a')) { flags |= TRACE_ARGS; }
  if (member(args[1], 'x')) { flags |= TRACE_EXEC; }
  if (member(args[1], 'h')) { flags |= TRACE_HEART_BEAT; }
  if (member(args[1], 'd')) { flags |= TRACE_APPLY; }
  if (member(args[1], 'p')) { flags |= TRACE_OBJNAME; }

  trace(flags);
  traceprefix(sizeof(args[0]) ? args[0][0] : 0);
  if (member(args[1], 'v')) {
    string traceprefix = traceprefix(0);
    traceprefix(traceprefix);
    int trace = trace(0);
    trace(trace);
    printf("        trace lfun calls (-l): %s\n"
           "       trace call_others (=o): %s\n"
           "  trace function returns (-r): %s\n"
           "trace function arguments (-a): %s\n"
           "      trace instructions (-x): %s\n"
           "         trace heartbeat (-h): %s\n"
           "    trace driver applies (-d): %s\n"
           "      print object names (-p): %s\n"
           "            trace prefix: %s\n",
           (trace &       TRACE_CALL ? "on" : "off"),
           (trace & TRACE_CALL_OTHER ? "on" : "off"),
           (trace &     TRACE_RETURN ? "on" : "off"),
           (trace &       TRACE_ARGS ? "on" : "off"),
           (trace &       TRACE_EXEC ? "on" : "off"),
           (trace & TRACE_HEART_BEAT ? "on" : "off"),
           (trace &      TRACE_APPLY ? "on" : "off"),
           (trace &    TRACE_OBJNAME ? "on" : "off"),
           (traceprefix ? traceprefix : "tracing all files")
          );
  }
  
  return 1;
}

