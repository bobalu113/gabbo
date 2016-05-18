/**
 * A generic CommandController module for commands added through the 
 * CommandGiver mixin. Each command must define a controller, and those
 * controllers may inherit this program to gain simplicity and uniformity
 * in handling command execution.
 *
 * @alias CommandController
 */
#include <sys/strings.h>
#include <sys/input_to.h>
#include <command.h>

private variables private functions inherit CommandLib;
private variables private functions inherit ArgsLib;
private variables private functions inherit StringsLib;
private variables private functions inherit StructLib;
private variables private functions inherit ObjectExpansionLib;
private variables private functions inherit FormatStringsLib;

struct CommandState {
  string verb;
  mixed *command;
  mixed *syntax;
  mapping opts;
  string *args;
  mapping extra;
  mapping model;
  int field_retry;
  int form_retry;
};

#define DEFAULT_PROMPT "(%t) %m%d{ [%s]}: "

int do_command(mixed *command, string verb, string arg);
int process_command(struct CommandState state, closure callback);
int process_args(struct CommandState state, closure callback);
int process_opts(struct CommandState state, mapping opts, closure callback);
int process_extra(struct CommandState state, closure callback);
int process_field(struct CommandState state, mixed *field, string field_type, mixed index, closure callback);
void field_prompt(struct CommandState state, mixed *field, string field_type, mixed index, closure callback);
void field_input(string arg, struct CommandState state, mixed *field, string field_type, mixed index, closure callback);
string parse_boolean(string arg, mixed val);
string parse_int(string arg, mixed val);
string parse_float(string arg, mixed val);
string parse_enum(string arg, mixed val, mixed *enum);
string parse_object(string arg, mixed val);
string parse_objects(string arg, mixed val);
int do_execute(mapping model, string verb);
int execute(mapping model, string verb);

closure prompt_formatter;

int do_command(mixed *command, string verb, string arg) {
  mapping opts, badopts;
  string *args;
  arg = trim(arg, TRIM_RIGHT, ' ');
  mixed *syntax = apply_syntax(command, arg, &opts, &badopts, &args);

  if (valid_syntax(syntax, opts, badopts, args)) {
    mapping model = ([ ]);
    struct CommandState state = 
      (<CommandState> verb, command, syntax, opts, args, ([ ]), model, 0, 0);
    return process_command(state, #'do_execute); //'
  } 

  // fail syntax
  return 0;
}

int process_command(struct CommandState state, closure callback) {
  if (!process_args(state, callback)) {
    return 0;
  }
  if (!process_opts(state, state->syntax[SYNTAX_OPTS], callback)) {
    return 0;
  }
  if (!process_opts(state, state->syntax[SYNTAX_LONGOPTS], callback)) {
    return 0;
  }
  if (!process_extra(state, callback)) {
    return 0;
  }

  // do form validation
  mixed *validations = state->syntax[SYNTAX_VALIDATION] 
                       + state->command[COMMAND_VALIDATION];
  foreach (mixed *validation : validations) {
    closure validator = symbol_function(validation[VALIDATE_VALIDATOR]);
    int result = apply(validator, state->model, validation[VALIDATE_PARAMS]);
    if (!result) {
      if (state->form_retry >= state->command[COMMAND_MAX_RETRY]) {
        // TODO print fail message
        return 0;
      } else {
        // TODO print fail message
        state->form_retry += 1;
        state->model = ([ ]);
        process_command(state, callback);
        return 0;
      }
    }
  }

  return funcall(callback, state->model, state->verb);
}

int process_args(struct CommandState state, closure callback) {
  int i = 0;
  int numargs = sizeof(state->args);
  foreach (mixed *field : state->syntax[SYNTAX_ARGS]) {
    if (!member(state->model, field[FIELD_ID])) {
      if (i < numargs) {
        // we have the arg, process it
        if (!process_field(state, field, "args", i, callback)) {
          return 0;
        }
      } else {
        // not enough args provided
        if ((field[FIELD_PROMPT_SETTING] == PROMPT_SYNTAX)
            || (field[FIELD_PROMPT_SETTING] == PROMPT_ALWAYS)) {
          // prompt user for value
          state->args += ({ field[FIELD_DEFAULT] });
          field_prompt(state, field, "args", i, callback);
          return 0;
        } else {
          if (field[FIELD_REQUIRED]) {
            // required field, fail usage
            return 0;            
          } else {
            // not required, continue with default
            state->args[i] = field[FIELD_DEFAULT];
            if (!process_field(state, field, "args", i, callback)) {
              return 0;
            }
          }
        }
      }
    }
    i++;
  }
  return 1;
}

int process_opts(struct CommandState state, mapping opts, closure callback) {
  foreach (mixed opt, mixed *field : opts) {
    if (!member(state->model, field[FIELD_ID])) {
      if (member(opts, opt)) {
        // we have the opt, process it
        if (!process_field(state, field, "opts", opt, callback)) {
          return 0;
        }
      } else {
        // opt not provided
        if ((field[FIELD_PROMPT_SETTING] == PROMPT_SYNTAX)
            || (field[FIELD_PROMPT_SETTING] == PROMPT_ALWAYS)) {
          // prompt user for value
          m_add(state->opts, opt, field[FIELD_DEFAULT]);
          field_prompt(state, field, "opts", opt, callback);
          return 0;
        } else {
          if (field[FIELD_REQUIRED]) {
            // required field, fail usage
            return 0;            
          } else {
            // not required, continue with default
            m_add(state->opts, opt, field[FIELD_DEFAULT]);
            if (!process_field(state, field, "opts", opt, callback)) {
              return 0;
            }
          }
        }
      }
    }
  }
  return 1;
}

int process_extra(struct CommandState state, closure callback) {
  foreach (mixed *field : state->command[COMMAND_FIELDS]) {
    string id = field[FIELD_ID];
    if (!member(state->model, id)) {
      if (member(state->extra, id)) {
        // we have the field, process it
        if (!process_field(state, field, "extra", id, callback)) {
          return 0;
        }
      } else {
        // field not provided
        if ((field[FIELD_PROMPT_SETTING] == PROMPT_SYNTAX)
            || (field[FIELD_PROMPT_SETTING] == PROMPT_ALWAYS)) {
          // prompt user for value
          m_add(state->extra, id, field[FIELD_DEFAULT]);
          field_prompt(state, field, "extra", id, callback);
          return 0;
        } else {
          if (field[FIELD_REQUIRED]) {
            // required field, fail usage
            return 0;            
          } else {
            // not required, continue with default
            m_add(state->extra, id, field[FIELD_DEFAULT]);
            if (!process_field(state, field, "extra", id, callback)) {
              return 0;
            }
          }
        }
      }
    }
  }
  return 1;
}

int process_field(struct CommandState state, mixed *field, string field_type, mixed index, closure callback) {
  mixed val;
  string fail;
  string id = field[FIELD_ID];
  string arg = get_struct_member(state, field_type)[index];
  switch (field[FIELD_TYPE]) {
    case "bool":
      fail = parse_boolean(arg, &val);
      break;
    case "int":
      fail = parse_int(arg, &val);
      break;
    case "float":
      fail = parse_float(arg, &val);
      break;
    case "enum":
      fail = parse_enum(arg, &val, field[FIELD_ENUM]);
      break;
    case "object":
      fail = parse_object(arg, &val);
      break;
    case "objects":
      fail = parse_objects(arg, &val);
      break;
    case "string":
    default:
      val = arg;
      break;
  }
  if (fail) {
    if (state->field_retry >= field[FIELD_MAX_RETRY]) {
      // TODO set fail message
      return 0;
    } else {
      // TODO print fail message
      state->field_retry += 1;
      field_prompt(state, field, field_type, index, callback);
      return 0;
    }
  } else {
    // do field validation
    foreach (mixed *validation : field[FIELD_VALIDATION]) {
      closure validator = symbol_function(validation[VALIDATE_VALIDATOR]);
      int result = apply(validator, val, validation[VALIDATE_PARAMS]);
      if (!result) {
        // validation failed
        if ((field[FIELD_PROMPT_SETTING] == PROMPT_VALIDATE)
            || (field[FIELD_PROMPT_SETTING] == PROMPT_ALWAYS)) {
          // prompt user for value
          field_prompt(state, field, field_type, index, callback);
          return 0;
        } else {
          // fail
        }
      }
    }
    // validation passed, add to model
    state->model[id] = val;
    // reset retries to 0 fo next field
    state->field_retry = 0;
    return 1;
  }
}

void field_prompt(struct CommandState state, mixed *field, string field_type, mixed index, closure callback) {
  int flags = INPUT_PROMPT;
  if (field[FIELD_PROMPT][PROMPT_NOECHO]) {
    flags |= INPUT_NOECHO;
  }

  string val = get_struct_member(state, field_type)[index];
  string prompt = funcall(
    prompt_formatter, 
    field[FIELD_PROMPT][PROMPT_MSG],
    (field[FIELD_TYPE] == "enum" 
      ? (field[FIELD_ENUM][ENUM_ID] 
        ? field[FIELD_ENUM][ENUM_ID]
        : "enum")
      : field[FIELD_TYPE]),
    val
  );
  input_to("field_input", flags, prompt, state, field, field_type, index, callback);
}

void field_input(string arg, struct CommandState state, mixed *field, string field_type, mixed index, closure callback) {
  get_struct_member(state, field_type)[index] = arg;
  process_command(state, callback);
}

string parse_boolean(string arg, mixed val) {
  switch (lower_case(arg)) {
    case "y":
    case "yes":
    case "true":
      val = 1;
      return 0;
    case "n":
    case "no":
    case "false":
      val = 0;
      return 0;
    default:
      return "Allowed values are: yes, no"; 
  }
  return 0;
}

string parse_int(string arg, mixed val) {
  if (sscanf("%d", arg, val)) {
    return 0;
  } else {
    return "Please enter a number.";
  }
  return 0;
}

string parse_float(string arg, mixed val) {
  int i, f;
  if (sscanf("%d.%d", arg, i, f)) {
    if (f == 0) {
      val = to_float(i);
    } else if (f == 1) {
      val = to_float(i) + 0.1;
    } else {
      val = to_float(i) + (to_float(f) / pow(10, ceil(log(f) / log(10))));
    }
    return 0;
  } else {
    return "Please enter a number.";
  }
  return 0;
}

string parse_enum(string arg, mixed val, mixed *enum) {
  mapping enum_map = mkmapping(enum);
  if (enum[ENUM_MULTI]) {
    val = ({ });
    string *args = explode_unescaped(arg, enum[ENUM_DELIM]);
    foreach (string a : args) {
      a = unescape(a);
      if (!member(enum_map, a)) {
        return "Allowed values are: " 
          + implode(enum[ENUM_VALUES], enum[ENUM_DELIM] + " ");
      }
      val += ({ a });
    }
  } else {
    if (!member(enum_map, arg)) {
      return "Allowed values are: " 
        + implode(enum[ENUM_VALUES], enum[ENUM_DELIM] + " ");
    }
    val = arg;
  }
  return 0;
}

string parse_object(string arg, mixed val) {
  val = expand_object(arg, THISP, 0);
  return 0;
}

string parse_objects(string arg, mixed val) {
  val = expand_objects(arg, THISP, 0);
  return 0;
}

int do_execute(mapping model, string verb) {
  return execute(model, verb);
}

int execute(mapping model, string verb) {
  return 0;
}

void create() {
  prompt_formatter = parse_format(DEFAULT_PROMPT, ([
      'm' : ({ 0, "%s", ({ ''message }) }),
      't' : ({ 0, "%s", ({ ''type }) }),
      'd' : ({ 0, "%s", ({ 
               ({ #'?, 
                  ({ #'stringp, ''default }), 
                  ({ #'sprintf, 'arg, ''default }),
                  ""
               })
            }) })
    ]),
    ({ 'message, 'type, 'default })
  ); //'
}
