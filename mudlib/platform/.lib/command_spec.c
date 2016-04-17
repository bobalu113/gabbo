/**
 * Library for parsing a command spec file.
 *
 * @author devo
 * @alias CommandSpecLib
 */

#define FieldParser         PlatformObjDir ""
#define TRUE_VALUE          "true"
#define FALSE_VALUE         "false"
#define DEFAULT_PARSER      FieldParser
#define DEFAULT_ENUM_MULTI  FALSE_VALUE
#define DEFAULT_REQUIRED    FALSE_VALUE
#define DEFAULT_DELIM       ","
#define DEFAULT_FORMAT      "explode"

mixed *parse_command_xml(string specfile, mixed *xml);
mixed *parse_fields_xml(string specfile, mixed *xml);
void parse_error(string specfile, string msg);

mixed *parse_command_xml(string specfile, mixed *xml) {
  string id;
  if (member(xml[XML_TAG_ATTRIBUTES], "id")) {
    id = xml[XML_TAG_ATTRIBUTES]["id"];
  }

  string parser = DEFAULT_PARSER;
  if (member(xml[XML_TAG_ATTRIBUTES], "parser")) {
    parser = xml[XML_TAG_ATTRIBUTES]["parser"];
  }

  string controller;
  if (member(xml[XML_TAG_ATTRIBUTES], "controller")) {
    controller = xml[XML_TAG_ATTRIBUTES]["controller"];
  } else {
    parse_error(specfile, "missing attribute controller");
  }

  string *verbs;
  if (member(xml[XML_TAG_ATTRIBUTES], "primaryVerb")) {
    verbs = ({ xml[XML_TAG_ATTRIBUTES]["primaryVerb"] });
  } else {
    parse_error(specfile, "missing attribute primaryVerb");
  }

  mixed *fields = ({ });
  mixed *syntax = ({ });
  mixed *validation = ({ });
  for (mixed *el : xml[XML_TAG_CONTENTS]) {
    switch (el[XML_TAG_NAME]) {
      case "fieldList":
        fields = parse_fields_xml(specfile, el[XML_TAG_CONTENTS]);
        break;
      case "argList":
        break;
      case "optSet":
        break;
      case "syntax":
        syntax += parse_syntax_xml(specfile, el[XML_TAG_CONTENTS]);
        break;
      case "validate":
        validation += parse_validate_xml(specfile, el[XML_TAG_CONTENTS]);
        break;
      case "subcommand":
        break;
      default:
        parse_error(specfile, "unknown element " + el[XML_TAG_NAME]);
    }
  }

  return ({ id, parser, controller, verbs, fields, syntax, validation });
}

mixed *parse_fields_xml(string specfile, mixed *xml) {
  mixed *result = ({ });
  for (mixed *el : xml[XML_TAG_CONTENTS]) {
    switch (el[XML_TAG_NAME]) {
      case "field":
        mixed *field = parse_field_xml(specfile, el[XML_TAG_CONTENTS]);
        if (field) {
          result += ({ field });
        }
        break;
      default:
        parse_error(specfile, "unknown element " + el[XML_TAG_NAME]);
        break;
    }
  }
  return result;
}

mixed *parse_field_xml(string specfile, mixed *xml) {
  string id;
  if (member(xml[XML_TAG_ATTRIBUTES], "id")) {
    id = xml[XML_TAG_ATTRIBUTES]["id"];
  } 

  string type;
  if (member(xml[XML_TAG_ATTRIBUTES], "type")) {
    type = xml[XML_TAG_ATTRIBUTES]["type"];
  } else {
    parse_error(specfile, "missing attribute type");
  }

  string required = DEFAULT_REQUIRED;
  if (member(xml[XML_TAG_ATTRIBUTES], "required")) {
    required = xml[XML_TAG_ATTRIBUTES]["required"];
  } 

  mixed *enum, *prompt;
  mixed *validation = ({ });
  for (mixed *el : xml[XML_TAG_CONTENTS]) {
    switch (el[XML_TAG_NAME]) {
      case "enum":
        enum = parse_enum_xml(specfile, el[XML_TAG_CONTENTS]);
        break;
      case "prompt":
        prompt = parse_prompt_xml(specfile, el[XML_TAG_CONTENTS]);
        break;
      case "validate":
        validation += ({ parse_validate_xml(specfile, el[XML_TAG_CONTENTS]) });
        break;
      default:
        parse_error(specfile, "unknown element " + el[XML_TAG_NAME]);
        break;
    }
  }

  return ({ id, type, required, enum, prompt, validate });
}

mixed *parse_enum_xml(string specfile, mixed *xml) {
  string id;
  if (member(xml[XML_TAG_ATTRIBUTES], "id")) {
    id = xml[XML_TAG_ATTRIBUTES]["id"];
  } 

  int multi = DEFAULT_ENUM_MULTI;
  if (member(xml[XML_TAG_ATTRIBUTES], "multi")) {
    multi = parse_boolean(xml[XML_TAG_ATTRIBUTES]["multi"]);
  } 

  string delim = DEFAULT_DELIM;
  if (member(xml[XML_TAG_ATTRIBUTES], "delim")) {
    delim = xml[XML_TAG_ATTRIBUTES]["delim"];
  } 

  mixed *values = ({ });
  for (mixed *el : xml[XML_TAG_CONTENTS]) {
    switch (el[XML_TAG_NAME]) {
      case "value":
        values += ({ parse_enum_value_xml(specfile, el[XML_TAG_CONTENTS]) });
        break;
      default:
        parse_error(specfile, "unknown element " + el[XML_TAG_NAME]);
        break;
    }
  }

  return ({ id, multi, delim, values });
}

mixed *parse_enum_value_xml(string specfile, mixed *xml) {
  string value;
  if (member(xml[XML_TAG_ATTRIBUTES], "value")) {
    value = xml[XML_TAG_ATTRIBUTES]["value"];
  } else {
    parse_error(specfile, "missing attribute value");
  }

  string label;
  if (member(xml[XML_TAG_ATTRIBUTES], "label")) {
    label = xml[XML_TAG_ATTRIBUTES]["label"];
  } 

  string type;
  if (member(xml[XML_TAG_ATTRIBUTES], "type")) {
    type = xml[XML_TAG_ATTRIBUTES]["type"];
  } else {
    parse_error(specfile, "missing attribute type");
  }

  return ({ value, label, type });
}

mixed *parse_prompt_xml(string specfile, mixed *xml) {
  int no_echo = DEFAULT_NO_ECHO;
  if (member(xml[XML_TAG_ATTRIBUTES], "noEcho")) {
    no_echo = parse_boolean(xml[XML_TAG_ATTRIBUTES]["noEcho"]);
  } 

  int show_abort = DEFAULT_SHOW_ABORT;
  if (member(xml[XML_TAG_ATTRIBUTES], "showAbort")) {
    show_abort = parse_boolean(xml[XML_TAG_ATTRIBUTES]["showAbort"]);
  } 

  int show_enum = DEFAULT_SHOW_ENUM;
  if (member(xml[XML_TAG_ATTRIBUTES], "showEnum")) {
    show_enum = parse_boolean(xml[XML_TAG_ATTRIBUTES]["showEnum"]);
  } 

  int max_retry = DEFAULT_MAX_RETRY;
  if (member(xml[XML_TAG_ATTRIBUTES], "maxRetry")) {
    max_retry = to_int(xml[XML_TAG_ATTRIBUTES]["maxRetry"]);
  } 

  string msg = xml[XML_TAG_CONTENTS];

  return ({ noecho, show_abort, show_enum, max_retry, msg });
}

mixed *parse_validate_xml(string specfile, mixed *xml) {
  string validator;
  if (member(xml[XML_TAG_ATTRIBUTES], "validator")) {
    validator = xml[XML_TAG_ATTRIBUTES]["validator"];
  } else {
    parse_error(specfile, "missing attribute validator");
  }

  string fail;
  if (member(xml[XML_TAG_ATTRIBUTES], "string")) {
    string = parse_boolean(xml[XML_TAG_ATTRIBUTES]["string"]);
  } 

  string *params = ({ });
  for (mixed *el : xml[XML_TAG_CONTENTS]) {
    switch (el[XML_TAG_NAME]) {
      case "param":
        values += ({ el[XML_TAG_ATTRIBUTES]["value"] });
        break;
      default:
        parse_error(specfile, "unknown element " + el[XML_TAG_NAME]);
        break;
    }
  }

  return ({ validator, fail, params });
}

mixed *parse_syntax_xml(string specfile, mixed *xml) {
  int args = -1;
  if (member(xml[XML_TAG_ATTRIBUTES], "args")) {
    args = to_int(xml[XML_TAG_ATTRIBUTES]["args"]);
  } 

  int min_args = -1;
  if (member(xml[XML_TAG_ATTRIBUTES], "minArgs")) {
    min_args = to_int(xml[XML_TAG_ATTRIBUTES]["minArgs"]);
  } 

  int max_args = -1;
  if (member(xml[XML_TAG_ATTRIBUTES], "maxArgs")) {
    max_args = to_int(xml[XML_TAG_ATTRIBUTES]["maxArgs"]);
  } 

  string pattern;
  if (member(xml[XML_TAG_ATTRIBUTES], "pattern")) {
    pattern = xml[XML_TAG_ATTRIBUTES]["pattern"];
  } else {
    parse_error(specfile, "missing attribute pattern");
  }

  string format = DEFAULT_FORMAT;
  if (member(xml[XML_TAG_ATTRIBUTES], "format")) {
    format = xml[XML_TAG_ATTRIBUTES]["format"];
  }

  mixed *args, *opts, *longopts;
  mixed *validation = ({ });
  for (mixed *el : xml[XML_TAG_CONTENTS]) {
    switch (el[XML_TAG_NAME]) {
      case "argList":
        break;
      case "optSet":
        break;
      case "opt":
        break;
      case "longopt":
        break;
      case "validate":
        validation += ({ parse_validate_xml(specfile, el[XML_TAG_CONTENTS]) });
        break;
      default:
        parse_error(specfile, "unknown element " + el[XML_TAG_NAME]);
        break;
    }
  }

  return ({ args, min_args, max_args, pattern, format, args, opts, longopts, validation });
}

void parse_error(string specfile, string msg) {
  raise_error(sprintf("Error parsing command spec %s: %s\n", specfile, msg));
}

