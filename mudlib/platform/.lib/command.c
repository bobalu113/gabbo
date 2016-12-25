/**
 * A library of functions related to the supporting the command line.
 *
 * @alias CommandLib
 */
#include <sys/regexp.h>
#include <command.h>

#define UNMATCHED_ARG  "__UNMATCHED_PARSE_COMMAND_ARG"

private variables private functions inherit StringsLib;
private variables private functions inherit ArgsLib;

mixed *apply_syntax(mixed *command, string arg, mapping opts, mapping badopts, 
                    string *args);
mixed *get_matching_syntax(mapping syntax_map, mapping opts, mapping badopts, 
                           mixed *args);
int valid_syntax(mixed *syntax, mapping opts, mapping badopts, mixed *args);
void parse_opts(string arg, int pos, mapping opts, mapping badopts, 
                mapping valid_opts, mapping valid_longopts);
void parse_longopt(string arg, int pos, string opt, string param, 
                   mapping badopts, mapping valid_longopts);
void parse_opt(string arg, int pos, mapping opts, mapping badopts, 
               mapping valid_opts);
string *parse_args(string arg, int pos, mapping syntax);
string *parse_args_explode(string arg, int pos, int numargs);
string *parse_args_sscanf(string arg, int pos, string pattern);
string *parse_args_regexp(string arg, int pos, string pattern);
string *parse_args_parse_command(string arg, int pos, string pattern);
void parse_param(string arg, int pos, string param);

private closure sscanf_args, parse_command_args;

mixed *apply_syntax(mixed *command, string arg, mapping opts, mapping badopts, 
                    string *args) {
  // TODO subcommands
  mapping syntax_map = ([ ]);
  foreach (mixed *syntax : command[COMMAND_SYNTAX]) {
    int pos = 0;

    // XXX cache these?
    object logger = LoggerFactory->get_logger(THISO);
    logger->info("%O", syntax);
    mapping valid_opts = mkmapping(map(syntax[SYNTAX_OPTS], 
                                       #'[, OPT_OPT),//'
                                   syntax[SYNTAX_OPTS]);
    mapping valid_longopts = mkmapping(map(syntax[SYNTAX_LONGOPTS], 
                                           #'[, OPT_OPT),//'
                                       syntax[SYNTAX_LONGOPTS]);

    opts = ([ ]);
    badopts = ([ ]);
    args = ({ });

    parse_opts(arg, &pos, &opts, &badopts, valid_opts, valid_longopts);
    pos = find_nonws(arg, pos);
    args = parse_args(arg, &pos, syntax);

    if (valid_syntax(syntax, opts, badopts, args)) {
      return syntax;
    } else {
      m_add(syntax_map, syntax, opts, badopts, args);
    }
  }

  mixed *syntax = get_matching_syntax(syntax_map, &opts, &badopts, &args);
  if (!syntax) {
    // no matching syntax, use first
    syntax = command[COMMAND_SYNTAX][0];
    opts = syntax_map[syntax, 0];
    badopts = syntax_map[syntax, 1];
    args = syntax_map[syntax, 2];
  }
  return syntax;
}

mixed *get_matching_syntax(mapping syntax_map, mapping opts, mapping badopts, 
                           mixed *args) {
  foreach (mixed *syntax, opts, badopts, args : syntax_map) {
    if (valid_syntax(syntax, opts, badopts, args)) {
      return syntax;
    }
  }

  foreach (mixed *syntax, opts, badopts, args : syntax_map) {
    if (valid_syntax(syntax, opts, ([ ]), args)) {
      return syntax;
    }
  }

  return 0;
}

int valid_syntax(mixed *syntax, mapping opts, mapping badopts, mixed *args) {
  // TODO make sure multi opts are valid
  int numargs = sizeof(args);
  if (syntax[SYNTAX_EXPLODE_ARGS] >= 0) {
    if (numargs == syntax[SYNTAX_EXPLODE_ARGS]) {
      if (!sizeof(badopts)) {
        return 1;
      }
    }
  } else {
    if (syntax[SYNTAX_MIN_ARGS] >= 0) {
      if (syntax[SYNTAX_MIN_ARGS] <= numargs) {
        if (syntax[SYNTAX_MAX_ARGS] >= 0) {
          if (syntax[SYNTAX_MAX_ARGS] >= numargs) {
            return (!sizeof(badopts));
          }
        } else {
          return (!sizeof(badopts));
        }
      }
    } else if (syntax[SYNTAX_MAX_ARGS] >= 0) {
      if (syntax[SYNTAX_MAX_ARGS] >= numargs) {
        return (!sizeof(badopts));
      }
    } else {
      return (!sizeof(badopts));
    }
  }
  return 0;
}

void parse_opts(string arg, int pos, mapping opts, mapping badopts, 
                mapping valid_opts, mapping valid_longopts) {
  int done = 0;
  int len = strlen(arg);
  while (!done && (pos < len)) {
    switch (arg[pos]) {
      case ' ':
      case '\t':
        pos++;
        break;
      case '-':
        pos++;
        if (pos >= len) {
          continue;
        }
        switch (arg[pos]) {
          case '-':
            pos++;
            string opt;
            string param;
            parse_longopt(arg, &pos, &opt, &param, &badopts, valid_longopts);
            if (opt) {
              if (!member(opts, opt)) {
                opts[opt] = ({ });
              }
              opts[opt] += ({ param });
            } else {
              // no more opts
              done = 1;
            } 
            break;
          case ' ':
            pos++;
            break;
          default:
            parse_opt(arg, &pos, &opts, &badopts, valid_opts);
            break;
        }
        break;
      default:
        // found an arg
        done = 1;
        break;
    }
  }
}

void parse_longopt(string arg, int pos, string opt, string param, 
                   mapping badopts, mapping valid_longopts) {
  opt = "";
  int len = strlen(arg);
  while ((pos < len) && (arg[pos] != ' ')) {
    opt = sprintf("%s%c", opt, arg[pos]);
    pos++;
  }

  if (member(valid_longopts, opt)) {
    if (valid_longopts[opt][OPT_PARAM]) {
      pos = find_nonws(arg, pos);
      if (pos < len) {
        parse_param(arg, &pos, &param);
      } else {
        param = "";
      }
    }
  } else {
    badopts[opt] = 1;
  }
}

void parse_opt(string arg, int pos, mapping opts, mapping badopts, 
               mapping valid_opts) {
  int len = strlen(arg);
  while ((pos < len) && (arg[pos] != ' ')) {
    int opt = arg[pos];
    pos++;
    if (member(valid_opts, opt)) {
      string param;
      if (valid_opts[opt][OPT_PARAM]) {
        pos = find_nonws(arg, pos);
        if (pos < len) {
          parse_param(arg, &pos, &param);
        }
      }
      if (!member(opts, opt)) {
        opts[opt] = ({ });
      }
      opts[opt] += ({ param });
      return;
    } else {
      badopts[opt] = 1;
    }
  }
}

string *parse_args(string arg, int pos, mapping syntax) {
  switch (syntax[SYNTAX_FORMAT]) {
    case "sscanf":
      return parse_args_sscanf(arg, &pos, syntax[SYNTAX_PATTERN]);
    case "regexp":
      return parse_args_regexp(arg, &pos, syntax[SYNTAX_PATTERN]);
    case "parse_command":
      return parse_args_parse_command(arg, &pos, syntax[SYNTAX_PATTERN]);
    case "explode":
    default:
      return parse_args_explode(arg, &pos, syntax[SYNTAX_EXPLODE_ARGS]);
  }
}

string *parse_args_explode(string arg, int pos, int numargs) {
  string *args = ({ });
  if (numargs >= 0) {
    // hard arg limit
    while (sizeof(args) < numargs) {
      pos = find_nonws(arg, pos);
      if (sizeof(args) == (numargs - 1)) {
        // last arg consume entire string
        int end = match_quote(arg, pos);
        if (end == strlen(arg)) {
          // quoted string, unquote/unescape
          string param = arg[pos..end];
          param = unquote(param);
          param = unescape(param);
          args += ({ param });
          pos = end + 1;
        } else {
          // unquoted, add verbatim
          args += ({ args[pos..] });
          pos = strlen(arg);
        }
      } else {
        // normal arg, parse out
        string param;
        parse_param(arg, &pos, &param);
        if (param) {
          args += ({ param });
        }
      }
    }
  } else {
    // no arg limit
    args = explode_args(arg);
    pos = strlen(arg);
  }
  return args;
}

string *parse_args_sscanf(string arg, int pos, string pattern) {
  if (!closurep(sscanf_args)) {
    mixed *sscanf_fragment = allocate(MAX_ARGS);
    mixed *eval_fragment = allocate(MAX_ARGS * 3);
    for (int i = 0; i < MAX_ARGS; i++) {
      sscanf_fragment[i] = quote("arg" + i);
      int j = i * 3;
      eval_fragment[j] = ({ i });
      eval_fragment[j + 1] = quote("arg" + 1);
      eval_fragment[j + 2] = #'break; //'
    }
    sscanf_args = lambda(({ 'arg, 'pattern }), 
      ({ #',, 
         ({ #'=, 
            'matches, 
            ({ #'sscanf, 'arg, 'pattern }) + sscanf_fragment 
         }), 
         ({ #'=, 'args, ({ #'allocate, 'matches }) }), 
         ({ #'=, 'i, 0 }), 
         ({ #'while, 
            ({ #'<, 'i, 'matches }), 
            'args, 
            ({ #'=, 
               ({ #'[, 'args, 'i }), 
               ({ #'switch, 'i }) + eval_fragment
            }), 
            ({ #'+=, 'i, 1 }) 
         }) 
      })
    ); //'
  }
  return funcall(sscanf_args, arg[pos..], pattern);
}

string *parse_args_regexp(string arg, int pos, string pattern) {
  string *args = regmatch(arg, pattern, RE_MATCH_SUBS, pos);
  return args[1..];
}

string *parse_args_parse_command(string arg, int pos, string pattern) {
  if (!closurep(parse_command_args)) {
    mixed *unmatched_fragment = allocate(MAX_ARGS);
    mixed *parse_command_fragment = allocate(MAX_ARGS);
    mixed *eval_fragment = allocate(MAX_ARGS * 3);
    for (int i = 0; i < MAX_ARGS; i++) {
      unmatched_fragment[i] = ({ #'=, quote("arg" + i), UNMATCHED_ARG }); //'
      parse_command_fragment[i] = quote("arg" + i);
      int j = i * 3;
      eval_fragment[j] = ({ i });
      eval_fragment[j + 1] = 
        ({ #'?, 
           ({ #'==, quote("arg" + i), UNMATCHED_ARG }), 
           ({ #'=, 'i, MAX_ARGS }), 
           ({ #'+=, 'args, quote("arg" + i) })
        });
      eval_fragment[j + 2] = #'break; //'
    }
    parse_command_args = lambda(({ 'arg, 'obs, 'pattern }), 
      ({ #',, 
         ({ #', }) + unmatched_fragment,
         ({ #'=, 
            'matched, 
            ({ #'parse_command, 'arg, 'obs, 'pattern }) + parse_command_fragment 
         }), 
         ({ #'?,
            'matched,
            ({ #',,
               ({ #'=, 'args, '({ }) }), 
               ({ #'=, 'i, 0 }), 
               ({ #'while, 
                  ({ #'<, 'i, MAX_ARGS }),
                  'args, 
                  ({ #'switch, 'i }) + eval_fragment, 
                  ({ #'+=, 'i, 1 }) 
               })
            }),
            '({ }),
         })
      })
    ); //'
  }
  return funcall(parse_command_args, arg[pos..], ({ }), pattern);
}

void parse_param(string arg, int pos, string param) {
  int end = match_quote(arg, pos);
  param = arg[pos..end];
  param = unquote(param);
  param = unescape(param);
  pos = end + 1;
}

