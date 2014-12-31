#ifndef _LOGGER_H
#define _LOGGER_H

#define Logger             ObjDir "/logger"

#define LOGGER_STALE_TIME  300
#define FACTORY_RESET_TIME 300
#define STANDING_REF_COUNT 3

#define PROP_PREFIX        "logger"
#define PROP_FILE          "etc/logger.properties"

#define ALLOWED_PROPS      ({ "output", "format", "level" })

#define LVL_ALL            "ALL"
#define LVL_TRACE          "TRACE"
#define LVL_DEBUG          "DEBUG"
#define LVL_INFO           "INFO"
#define LVL_WARN           "WARN"
#define LVL_ERROR          "ERROR"
#define LVL_FATAL          "FATAL"
#define LVL_OFF            "OFF"
#define LEVELS             ([ LVL_ALL    : 7, \
                              LVL_TRACE  : 6, \
                              LVL_DEBUG  : 5, \
                              LVL_INFO   : 4, \
                              LVL_WARN   : 3, \
                              LVL_ERROR  : 2, \
                              LVL_FATAL  : 1, \
                              LVL_OFF    : 0 ])

#define OUT_CONSOLE        'c'
#define OUT_FILE           'f'

#define DEFAULT_FORMAT     "%d{%Y-%m-%d %H:%M:%S},%r %p %l - %m"
#define DEFAULT_LEVEL      LVL_OFF


#define FMT_NEWLINE   ({ 0, "\n", ({ }) })
#define FMT_PERCENT   ({ 0, "%%", ({ }) })
#define FMT_CATEGORY  ({ "0", "%s", ({                           \
  ({ #'?,                                                        \
     'arg,                                                       \
     ({ #'implode,                                               \
        ({ #'[<.., ({ #'explode, ''category, "." }), 'arg }),    \
        "."                                                      \
     }),                                                         \
     ''category                                                  \
  })                                                             \
}) })
#define FMT_OBJECT    ({ "0", "%s", ({                           \
  ({ #'?,                                                        \
     'arg,                                                       \
     ({ #'implode,                                               \
        ({ #'[<..,                                               \
           ({ #'explode,                                         \
              ({ #'?,                                            \
                 ''caller,                                       \
                 ({ #'[, ''caller, TRACE_PROGRAM }),             \
                 ""                                              \
              }),                                                \
              "/"                                                \
           }),                                                   \
           ({ #'to_int, 'arg })                                  \
        }),                                                      \
        "."                                                      \
     }),                                                         \
     ({ #'?,                                                     \
        ''caller,                                                \
        ({ #'[, ''caller, TRACE_PROGRAM }),                      \
        ""                                                       \
     })                                                          \
  })                                                             \
}) })
#define FMT_DATE      ({ "%F %T", "%s", ({                       \
  ({ #'||, ({ #'strftime, 'arg, ({ #'time }) }), "" })           \
}) })
#define FMT_LOCATION  ({ 0, "%s", ({                             \
  ({ #'?,                                                        \
     ''caller,                                                   \
     ({ #'sprintf,                                               \
        "%s->%s(%s:%d)",                                         \
        ({ #'[, ''caller, TRACE_OBJECT }),                       \
        ({ #'[, ''caller, TRACE_NAME }),                         \
        ({ #'[, ''caller, TRACE_PROGRAM }),                      \
        ({ #'[, ''caller, TRACE_LOC })                           \
     }),                                                         \
     ""                                                          \
  })                                                             \
}) })
#define FMT_LINE      ({ 0, "%d", ({                             \
  ({ #'?,                                                        \
     ''caller,                                                   \
     ({ #'[, ''caller, TRACE_LOC }),                             \
     -1                                                          \
  })                                                             \
}) })
#define FMT_MSG       ({ 0, "%s", ({ ''message }) })
#define FMT_PRIORITY  ({ 0, "%s", ({                             \
  ({ #'sprintf, "%-5s", ''priority })                            \
}) })
#define FMT_MILLIS    ({ 0, "%d", ({                             \
  ({ #'[, ({ #'utime }), 1 })                                    \
}) })

#define LOGGER_FORMAT ([ \
  'n' : FMT_NEWLINE,     \
  '%' : FMT_PERCENT,     \
  'c' : FMT_CATEGORY,    \
  'C' : FMT_OBJECT,      \
  'd' : FMT_DATE,        \
  'l' : FMT_LOCATION,    \
  'L' : FMT_LINE,        \
  'm' : FMT_MSG,         \
  'p' : FMT_PRIORITY,    \
  'r' : FMT_MILLIS       \
])

#endif  // _LOGGER_H
