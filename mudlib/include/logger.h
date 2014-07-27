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
   
#define FMT_CATEGORY       'c'
#define FMT_OBJECT         'C'
#define FMT_DATE           'd'
#define FMT_LOCATION       'l'
#define FMT_LINE           'L'
#define FMT_MSG            'm'
#define FMT_NEWLINE        'n'
#define FMT_PRIORITY       'p'
#define FMT_MILLIS         'r'
#define FMT_PERCENT        '%'
   
#define OUT_CONSOLE        'c'
#define OUT_FILE           'f'

#define DEFAULT_FORMAT     "%d{%Y-%m-%d %H:%M:%S},%r %p %l - %m"
#define DEFAULT_LEVEL      LVL_OFF
   
#endif  // _LOGGER_H
