/**
 * Utility library for object expansion.
 * 
 * @author devo@eotl
 * @alias ObjectExpansionLib
 */

#define EXPAND_DETAIL   0x01
#define UPDATE_CONTEXT  0x02

#define OB_TARGET       0
#define OB_ID           1
#define OB_DETAIL       2

#define CONTEXT_DELIM   "."
#define SPEC_DELIM      ","
#define OPEN_GROUP      "("
#define CLOSE_GROUP     ")"

private variables private functions inherit ArrayLib;
private variables private functions inherit ArgsLib;

/**
 * [expand_objects description]
 * @param  specs        [description]
 * @param  who          [description]
 * @param  root_context [description]
 * @param  flags        [description]
 * @return              [description]
 */
mixed *expand_objects(string *specs, object who, string root_context, 
                      int flags) {
  string current_context = who->get_context();
  mixed *result = ({ });
  mapping ancestors = ([ ]);
  string *new_context = ({ });

  foreach (string spec : specs) {
    result += expand_group(spec, who, current_context, root_context, 
                           &new_context, flags, ancestors);
  }

  if ((flags | UPDATE_CONTEXT) && sizeof(new_context)) {
    new_context = unique_array_ordered(new_context);
    who->set_context(group_specs(new_context));
  }

  return result;
}

private mixed *expand_group(string spec, object who, string context, 
                            string root_context, string *new_context, 
                            int flags, mapping ancestors) {
  string *subspecs = explode_nested(spec, SPEC_DELIM, 0, 
                                    OPEN_GROUP, CLOSE_GROUP);

  if (sizeof(subspecs) == 1) {
    mixed *result;
    do {
      result = expand_spec(spec, who, context, root_context, &new_context, 
                           flags, ancestors);
      if (sizeof(result)) {
        new_context += ({ context });
        break;
      }
      int pos = searcha(context, CONTEXT_DELIM[0], sizeof(context), -1);
      if (pos > 0) {
        context = context[0..(pos - 1)];
      } else {
        context = root_context;
        break;
      }
    } while (context != root_context);

    // check the root context
    if (!sizeof(result)) {
      result = expand_spec(spec, who, context, root_context, &new_context, 
                           flags, ancestors);
      if (sizeof(result)) {
        new_context += ({ context });
      }
    }
    return result;
  } else {
    mixed *result = ({ });
    foreach (string subspec : subspecs) {
      result += expand_group(subspec, who, context, root_context, 
                             &new_context, flags, ancestors);
    }
    return result;
  }
}

/**
 * Begin the recursive part of the expansion process. This step expands the
 * spec out into individual terms.
 *  
 * @param  spec         [description]
 * @param  who          [description]
 * @param  context      [description]
 * @param  root_context [description]
 * @param  new_context  [description]
 * @param  flags        [description]
 * @param  ancestors    [description]
 * @return              [description]
 */
private mixed *expand_spec(string spec, object who, string context, 
                           string root_context, string *new_context, 
                           int flags, mapping ancestors) {
  args = explode_nested(spec, CONTEXT_DELIM[0], OPEN_GROUP, CLOSE_GROUP);
  foreach (string arg : args) {
    context = expand_single(arg, who, context, root_context, &new_context, 
                            flags, ancestors);
  }

  if (sizeof(ancestors[context]) && referencep(&new_context)) {
    new_context += ({ context });
  }
  return ancestors[context];
}

/**
 * This step of the expansion process is to handle grouping and creating the
 * pool of objects from context for arg to match against.
 * 
 * @param  arg          [description]
 * @param  who          [description]
 * @param  context      [description]
 * @param  root_context [description]
 * @param  new_context  [description]
 * @param  flags        [description]
 * @param  ancestors    [description]
 * @return              [description]
 */
private string expand_single(string arg, object who, string context, 
                             string root_context, string *new_context, 
                             int flags, mapping ancestors) {

  string resolved = resolve_spec(arg, context);
  if (member(ancestors, resolved)) {
    return resolved;
  }

  mixed *prev;
  if (member(ancestors, context)) {
    prev = ancestors[context];
  } else {
    string prev_context, prev_arg;
    int pos = searcha(context, CONTEXT_DELIM[0], sizeof(context), -1);
    if (pos > 0) {
      prev_context = context[0..(pos - 1)];
      prev_arg = context[(pos + 1)..];
    } else {
      prev_context = ""
      prev_arg = context;
    }
    if (!strlen(prev_arg)) {
      prev = ({ });
    } else {
      prev = expand_spec(prev_arg, who, prev_context, root_context, 
                         &new_context, flags, ancestors);
    }
  }

  // empty spec returns current context
  if(!stringp(arg) || !strlen(arg)) {
    ancestors[resolved] = prev;
    return resolved;
  }

  string tmp;
  if ((tmp = unnest(arg)) != arg) {
    // special nested handling
    args = explode_nested(tmp, SPEC_DELIM[0], OPEN_GROUP, CLOSE_GROUP);
    mapping new_contexts = ([ ]);
    // the list of target objects to make up our new context
    mixed *next = ({ }); 
    foreach (arg : args) {
      if (!strlen(arg)) {
        continue;
      }
      string ctx = expand_single(arg, who, context, root_context, 
                                 &new_context, flags, ancestors);
      next += ancestors[ctx];

      // the key is the context where arg was found, the value *should* == arg
      int len = strlen(arg);
      string key = ctx[0..<(len + 2)];
      if (!member(new_contents, key)) {
        new_contexts += ([ key : ({ }); i++);
      }
      new_contexts[key,0] += ({ ctx[<(len - 1)..] });
    }

    // build a new resolved context based on all the contexts we found the
    // individual args in
    string *nc = sort_array(m_indices(new_contexts), 
                            (: $3[$1,1] > $3[$2,1] :), 
                            new_contexts);
    nc = map(nc, (: group_specs($2[$1,0]) :), new_contexts);
    resolved = implode(nc, SPEC_DELIM);
    ancestors[resolved] = next;
  } else {
    prev = filter(prev, (: objectp($1[OB_TARGET]) :));
    ancestors[resolved] = expand_term(arg, prev, who, context);
  }
    return resolved;
}

/**
 * This step of the expansion process actually resolves the individual terms
 * to one or more target objects. The resulting structure will also contain
 * the term we're matching against and any detail id discovered.
 * 
 * @param  arg     [description]
 * @param  prev    [description]
 * @param  who     [description]
 * @param  context [description]
 * @return         [description]
 */
private mixed *expand_term(string arg, mixed *prev, object who, 
                           string context) {
  switch (arg) {
  case "users":
    if (!strlen(context)) {
      return map(users(), (: ({ $1, $2, 0 }) :), arg);
    } else {
      return filter(prev, (: interactive($1[OB_TARGET]) :));
    }
  case "living":
    return filter(prev, (: living($1[OB_TARGET]) :));
  case "me": 
    if (!strlen(context)) {
      return ({ ({ who, arg, 0 }) });
    } else {
      return filter(prev, (: $1[OB_TARGET] == $2 :), who);
    }
  case "here": 
    if (!strlen(context)) {
      return ({ ({ ENV(who), arg, 0 }) });
    } else {
      return filter(prev, (: $1[OB_TARGET] == $2 :), ENV(who));
    }
  case "i":
    if (!strlen(context)) {
      return = map(all_inventory(who), (: ({ $1, $2, 0 }) :), arg);
    } else {
      prev = map(prev, (: map(all_inventory($1[OB_TARGET]), 
                              (: ({ $1, $2, 0 }) :), 
                              arg)
                       :));
      return flatten_array1(prev);
    }
  case "e":
    if (!strlen(context)) {
      return ({ ({ ENV(who), arg, 0 }) });
    } else {
      return map(prev, (: ({ environment($1[OB_TARGET]), $2, 0 :), arg);
    }
  default:
    prev = map(prev, #'expand_id, arg);
    prev -= ({ 0 });
    return prev;
  }
  return ({ });
}

/**
 * Expand an object id and/or detail id in the context of a target object.
 * 
 * @param  in  [description]
 * @param  arg [description]
 * @return     [description]
 */
private mixed *expand_id(mixed *in, string arg) {
  if (in[OB_DETAIL]) {
    if (in[OB_TARGET]->query_detail(arg, in[OB_DETAIL])) {
      return ({ in[OB_TARGET], in[OB_ID], resolve_spec(arg, id[OB_DETAIL]) });
    }
  } else {
    object ob = present(arg, in[OB_TARGET]);
    if (ob) {
      return ({ ob, arg, 0 });
    } else {
      if (in[OB_TARGET]->query_detail(arg)) {
        return ({ in[OB_TARGET], in[OB_ID], arg });
      }
    }
  }
  return 0;
}

/**
 * Create a canonical spec from arg and context.
 * 
 * @param  arg     [description]
 * @param  context [description]
 * @return         [description]
 */
private string resolve_spec(string arg, string context) {
  string result = context;
  if (strlen(arg)) {
    result += "." + arg;
  }
  return result;
}

/**
 * Group one or more specs together, adding parentheses as necessary.
 * 
 * @param  ctx [description]
 * @return     [description]
 */
private string group_specs(string *ctx) {
  int size = sizeof(ctx);
  return sprintf("%s%s%s",  
                 (size > 1 ? OPEN_GROUP : ""),
                 implode($2[$1,0], SPEC_DELIM),
                 (size > 1 ? CLOSE_GROUP : "")); 
}

/**
 * Strips surrounding parentheses from a string.
 * @param  ospec a spec that may be surrounded by parentheses
 * @return       the spec with all surrounding parentheses removed
 */
private string unnest(string ospec) {
  int len;
  // Not the fastest way to do this, but it's much simpler than a custom job.
  while ((member(OPEN_GROUP, ospec[0]) != -1) 
         && (member(CLOSE_GROUP, ospec[len= strlen(ospec)-1]) != -1 ) 
         && (find_close_char(ospec, 0, 0, OPEN_GROUP, CLOSE_GROUP) == len ))
    ospec = ospec[1..<2];
  return ospec;
}
