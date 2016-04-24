#ifndef _COMMAND_H
#define _COMMAND_H

#define COMMAND_ID            0
#define COMMAND_CONTROLLER    1
#define COMMAND_VERBS         2
#define COMMAND_FIELDS        3
#define COMMAND_SYNTAX        4
#define COMMAND_VALIDATION    5

#define FIELD_ID              0
#define FIELD_TYPE            1
#define FIELD_REQUIRED        2
#define FIELD_ENUM            3
#define FIELD_PROMPT          4
#define FIELD_VALIDATION      5

#define OPTS_OPTS             0
#define OPTS_LONGOPTS         1
#define OPT_OPT               6
#define OPT_PARAM             7
#define OPT_MULTI             8

#define ENUM_ID               0
#define ENUM_MULTI            1
#define ENUM_DELIM            2
#define ENUM_VALUES           3

#define ENUM_VALUE_VALUE      0
#define ENUM_VALUE_LABEL      1
#define ENUM_VALUE_TYPE       2

#define PROMPT_NO_ECHO        0
#define PROMPT_SHOW_ABORT     1
#define PROMPT_SHOW_ENUM      2
#define PROMPT_MAX_RETRY      3
#define PROMPT_MSG            4

#define VALIDATE_VALIDATOR    0
#define VALIDATE_FAIL         1
#define VALIDATE_PARAMS       2

#define SYNTAX_EXPLODE_ARGS   0
#define SYNTAX_MIN_ARGS       1
#define SYNTAX_MAX_ARGS       2
#define SYNTAX_PATTERN        3
#define SYNTAX_FORMAT         4
#define SYNTAX_ARGS           5
#define SYNTAX_OPTS           6
#define SYNTAX_LONGOPTS       7
#define SYNTAX_VALIDATION     8
#define SYNTAX_SUBCOMMANDS    9

#endif  // _COMMAND_H
