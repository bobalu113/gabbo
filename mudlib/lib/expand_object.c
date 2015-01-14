/**
 * Utility library for object expansion.
 *
 * @author devo@eotl
 * @alias ObjectExpansionLib
 */

#include <expand_object.h>

private variables private functions inherit ArrayLib;
private variables private functions inherit ArgsLib;
private variables private functions inherit FileLib;

private mixed *expand_group(string ospec, object who, string context,
                            string root_context, string *new_context,
                            int flags, mapping ancestors);
private mixed *expand_spec(string ospec, object who, string context,
                           string *new_context, int flags,
                           mapping ancestors);
private string expand_single(string arg, object who, string context,
                             string *new_context, int flags,
                             mapping ancestors);
private mixed *expand_term(string term, mixed *prev, object who,
                           string context, int flags);
private mixed *expand_id(mixed *in, string id);
private string resolve_spec(string arg, string context);
private string group_specs(string *ospecs);
private string unnest(string ospec);
private int valid_environment(object arg);
private object find_room(object arg);

/**
 * Expand one or more object specifiers into a list of matching target
 * objects. Each specifier will be resolved in the context of a player or
 * some other object, e.g. a ospec of "e" resolves to ENV(who). You may also
 * pass a root context in the form of some other specifier. If the parser is
 * unable to find any matching objects based on 'who', it will try to find a
 * matching object in the expanded root context. Lastly, a bitvector of
 * various flags may be passed in to control parser behavior.
 *
 * @param  ospecs       an array of ospecs to expand
 * @param  who          the context in which to perform the expansion
 * @param  root_context an optional object ospec which will be used if no
 *                      objects can be found for 'who'
 * @param  flags        control flags
 * @return              an array of target objects, expressed as:
 *                      <code>({ ob, id, detail })</code>, where 'ob' is the
 *                      target object which matched the ospec, 'id' is the
 *                      term the object matched on, and 'detail' is the
 *                      detail id in ob which matched the ospec, or 0 if no
 *                      detail was specified
 */
varargs mixed *expand_objects(mixed ospecs, object who,
                              string root_context, int flags) {
  string current_context = who->query_context() || "";
  mixed *result = ({ });
  mapping ancestors = ([ ]);
  string *new_context = ({ });

  if (stringp(ospecs)) {
    ospecs = ({ ospecs });
  }
  if (!pointerp(ospecs)) {
    return ({ });
  }
  if (!stringp(root_context)) {
    root_context = "";
  }

  foreach (string ospec : ospecs) {
    if (!stringp(ospec)) {
      continue;
    }
    result += expand_group(ospec, who, current_context, root_context,
                           &new_context, flags, ancestors);
    if (sizeof(result) && (flags & LIMIT_ONE)) {
      break;
    }
  }

  if ((flags | UPDATE_CONTEXT) && sizeof(new_context)) {
    new_context = unique_array(new_context);
    who->set_context(group_specs(new_context));
  }

  if (sizeof(result) && (flags & LIMIT_ONE)) {
    return ({ result[0] });
  } else {
    return result;
  }
}

/**
 * Process the individual ospecs from the list passed to expand_objects().
 * This function is processes grouped ospecs, splitting them up and sending
 * them off to expand_spec().
 *
 * @param  ospec        the spec to be expanded
 * @param  who          the subject doing the expanding
 * @param  context      the current context in which to look for target
 *                      objects from ospec
 * @param  root_context the root context to use if no targets found in
 *                      current context
 * @param  new_context  an array, passed by reference, to report the contexts
 *                      where we actually found the target objects. This will
 *                      be deduped and grouped into a single resolved context
 *                      at the end
 * @param  flags        control flags
 * @param  ancestors    a mapping of contexts we've found along the way,
 *                      mapped to the list of matching target objects
 * @return              the list of target objects matching this spec
 *                      (see expand_objects())
 */
private mixed *expand_group(string ospec, object who, string context,
                            string root_context, string *new_context,
                            int flags, mapping ancestors) {
  string *subspecs = explode_nested(ospec, SPEC_DELIM,
                                    OPEN_GROUP, CLOSE_GROUP);

  if (sizeof(subspecs) == 1) {
    mixed *result;
    do {
      result = expand_spec(ospec, who, context, &new_context, flags,
                           ancestors);
      // found matching object in this context
      if (sizeof(result)) {
        new_context += ({ context });
        break;
      }
      int pos = searcha(context, CONTEXT_DELIM[0], sizeof(context), -1);
      if (pos > 0) {
        // check the next context up the path
        context = context[0..(pos - 1)];
      } else {
        // path empty, check the root context
        context = root_context;
        break;
      }
    } while (context != root_context);

    // check the root context
    if (!sizeof(result)) {
      result = expand_spec(ospec, who, context, &new_context, flags,
                           ancestors);
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
      if (sizeof(result) && (flags & LIMIT_ONE)) {
        break;
      }
    }
    return result;
  }
}

/**
 * This step expands the ospec out into individual terms to be processed by
 * expand_single().
 *
 * @param  ospec        the spec to be expanded
 * @param  who          the subject doing the expanding
 * @param  context      the current context in which to look for target
 *                      objects from ospec
 * @param  new_context  an array, passed by reference, to report the contexts
 *                      where we actually found the target objects. This will
 *                      be deduped and grouped into a single resolved context
 *                      at the end
 * @param  flags        control flags
 * @param  ancestors    a mapping of contexts we've found along the way,
 *                      mapped to the list of matching target objects
 * @return              the list of target objects matching this spec
 *                      (see expand_objects())
 */
private mixed *expand_spec(string ospec, object who, string context,
                           string *new_context, int flags,
                           mapping ancestors) {
  string *args = explode_nested(ospec, CONTEXT_DELIM,
                                OPEN_GROUP, CLOSE_GROUP);
  foreach (string arg : args) {
    context = expand_single(arg, who, context, &new_context, flags,
                            ancestors);
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
 * @param  arg          the spec to be expanded
 * @param  who          the subject doing the expanding
 * @param  context      the current context in which to look for target
 *                      objects from ospec
 * @param  new_context  an array, passed by reference, to report the contexts
 *                      where we actually found the target objects. This will
 *                      be deduped and grouped into a single resolved context
 *                      at the end
 * @param  flags        control flags
 * @param  ancestors    a mapping of contexts we've found along the way,
 *                      mapped to the list of matching target objects
 * @return              the new context where in ancestors where the matching
 *                      target objects may be found
 */
private string expand_single(string arg, object who, string context,
                             string *new_context, int flags,
                             mapping ancestors) {

  string resolved = resolve_spec(arg, context);
  if (member(ancestors, resolved)) {
    return resolved;
  }

  mixed *prev;
  if (member(ancestors, context)) {
    prev = ancestors[context];
  } else {
    string prev_context, prev_arg;
    int pos = searcha(context, CONTEXT_DELIM[0], sizeof(context) - 1, -1);
    if (pos > 0) {
      prev_context = context[0..(pos - 1)];
      prev_arg = context[(pos + 1)..];
    } else {
      prev_context = "";
      prev_arg = context;
    }
    if (!strlen(prev_arg)) {
      prev = ({ });
    } else {
      prev = expand_spec(prev_arg, who, prev_context, &new_context, flags,
                         ancestors);
    }
  }

  // empty ospec returns current context
  if(!stringp(arg) || !strlen(arg)) {
    ancestors[resolved] = prev;
    return resolved;
  }

  string tmp;
  if ((tmp = unnest(arg)) != arg) {
    // special nested handling
    string *args = explode_nested(tmp, SPEC_DELIM,
                                  OPEN_GROUP, CLOSE_GROUP);
    mapping new_contexts = ([ ]);
    int i = 0;
    // the list of target objects to make up our new context
    mixed *next = ({ });
    foreach (arg : args) {
      if (!strlen(arg)) {
        continue;
      }
      string ctx = expand_single(arg, who, context, &new_context, flags,
                                 ancestors);
      next += ancestors[ctx];

      // the key is the context where arg was found, the value *should* == arg
      int len = strlen(arg);
      string key = ctx[0..<(len + 2)];
      if (!member(new_contexts, key)) {
        new_contexts += ([ key : ({ }); i++ ]);
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
    ancestors[resolved] = expand_term(arg, prev, who, context, flags);
  }
    return resolved;
}

/**
 * This step of the expansion process actually resolves the individual terms
 * to one or more target objects. The resulting structure will also contain
 * the term we're matching against and any detail id discovered.
 *
 * @param  arg     one individual term inside the ospec
 * @param  prev    the list of objects which matched the previous term
 * @param  who     the subject doing the expanding
 * @param  context the current context in which to look for target
 *                 objects from ospec
 * @param  flags   control flags
 * @return         the list of matching target objects
 */
private mixed *expand_term(string term, mixed *prev, object who,
                           string context, int flags) {
  object logger = LoggerFactory->get_logger(THISO);
  switch (term) {
  case "users":
    if (!strlen(context)) {
      return map(users(), (: ({ $1, "users", 0 }) :));
    } else {
      return filter(prev, (: interactive($1[OB_TARGET]) :));
    }
  case "living":
    return filter(prev, (: living($1[OB_TARGET]) :));
  case "me":
    if (!strlen(context)) {
      return ({ ({ who, "me ", 0 }) });
    } else {
      return filter(prev, (: $1[OB_TARGET] == $2 :), who);
    }
  case "here":
    if (!strlen(context)) {
      return ({ ({ ENV(who), "here", 0 }) });
    } else {
      return filter(prev, (: $1[OB_TARGET] == $2 :), ENV(who));
    }
  case "i":
    if (!strlen(context)) {
      return map(all_inventory(who), (: ({ $1, "i", 0 }) :));
    } else {
      prev = map(prev, (: map(all_inventory($1[OB_TARGET]),
                              (: ({ $1, "", 0 }) :))
                       :));
      return flatten_array1(prev);
    }
  case "e":
    if (!strlen(context)) {
      return ({ ({ ENV(who), "e", 0 }) });
    } else {
      return map(prev, (: ({ environment($1[OB_TARGET]),
                             $1[OB_ID],
                             $1[OB_DETAIL]
                          }) :));
    }
  default:
    // first look for matching ids
    logger->trace("term = %O, prev = %O", term, prev);
    mixed *id_matches = map(prev, #'expand_id, term); //'
    logger->trace("id_matches = %O", id_matches);
    id_matches -= ({ 0 });
    if (sizeof(id_matches)) {
      prev = id_matches;
    } else {
      // look for a matching object name
      object *matches = ({ });
      object exact_match = FINDO(term);
      if (exact_match) {
        matches += ({ exact_match });
      } else {
        // look for matching program names and get their clones
        mixed *files = expand_pattern(term, who);
        logger->trace("files = %O\n", files);
        foreach (mixed *f : files) {
          string file = f[0];
          if (!is_loadable(file)) {
            continue;
          }
          object ob;
          string ret = catch (ob = load_object(file));
          if (ret || !ob) {
            continue;
          }
          if (flags & MATCH_BLUEPRINTS) {
            matches += ({ ob });
          }
          matches += clones(ob, ((flags & STALE_CLONES) ? 2 : 0));
        }
      }
      logger->trace("matches = %O\n", matches);
      if (!strlen(context)) {
        return map(matches, (: ({ $1, term, 0 }) :));
      } else {
        prev = filter(prev,
                      (: member($2, $1[OB_TARGET]) :),
                      mkmapping(matches));
      }
    }
    return prev;
  }
  return ({ });
}

/**
 * Expand an object id and/or detail id in the context of a target object.
 *
 * @param  in a target object in which to look for an object with the
 *            provided id
 * @param  id the id to look for
 * @return    a new target object with id/detail id expanded
 */
private mixed *expand_id(mixed *in, string id) {
  LoggerFactory->get_logger(THISO)->trace("expand_id, id= %O, in = %O", id, in);
  if (in[OB_DETAIL]) {
    if (in[OB_TARGET]->query_detail(id, in[OB_DETAIL])) {
      return ({ in[OB_TARGET], in[OB_ID], resolve_spec(id, in[OB_DETAIL]) });
    }
  } else {
    object ob = present(id, in[OB_TARGET]);
    if (ob) {
      return ({ ob, id, 0 });
    } else {
      if (in[OB_TARGET]->query_detail(id)) {
        return ({ in[OB_TARGET], in[OB_ID], id });
      }
    }
  }
  return 0;
}

/**
 * Expand a destination string, which can be an object expression or a file
 * pattern. If arg is applied as a file pattern and it refers to a a valid
 * room or container, this function will attempt to load/clone the room.
 *
 * @param  arg          the destination specifier
 * @param  who          object
 * @param  root_context an optional object ospec which will be used if no
 *                      objects can be found for 'who'
 * @param  flags        control flags
 * @param  error        passed by reference, will be assigned an error string
 * @return              the destination object, or 0 if no destination was
 *                      found
 */
varargs object expand_destination(string arg, object who,
                                  string root_context, int flags,
                                  string error) {
  object dest;

  mixed *targets = expand_objects(arg, who, root_context, flags);
  if (sizeof(targets)) {
    foreach (mixed *t : targets) {
      dest = find_room(t[OB_TARGET]);
      if (!dest) {
        error = "not inside a valid room or container";
        continue;
      }
      break;
    }
  } else {
    string *files = expand_pattern(arg, who);
    foreach (mixed *f : files) {
      string file = f[0];
      if (!is_loadable(file)) {
        continue;
      }
      dest = load_object(file);
      if (!dest) {
        error = "error loading object";
        continue;
      }
      break;
    }
  }

  return dest;
}

/**
 * Create a canonical ospec from arg and context.
 *
 * @param  arg     the argument being searched
 * @param  context the context where in which the argument was searched
 * @return         a resolved spec, including context
 */
private string resolve_spec(string arg, string context) {
  string result = context;
  if (strlen(arg)) {
    result += CONTEXT_DELIM + arg;
  }
  return result;
}

/**
 * Group one or more ospecs together, adding parentheses as necessary.
 *
 * @param  ospecs the ospecs to group together
 * @return        the new compound ospec
 */
private string group_specs(string *ospecs) {
  int size = sizeof(ospecs);
  return sprintf("%s%s%s",
                 (size > 1 ? OPEN_GROUP : ""),
                 implode(ospecs, SPEC_DELIM),
                 (size > 1 ? CLOSE_GROUP : ""));
}

/**
 * Strips surrounding parentheses from a string.
 * @param  ospec a ospec that may be surrounded by parentheses
 * @return       the ospec with all surrounding parentheses removed
 */
private string unnest(string ospec) {
  int len;
  // Not the fastest way to do this, but it's much simpler than a custom job.
  while ((member(OPEN_GROUP, ospec[0]) != -1)
         && (member(CLOSE_GROUP, ospec[len= strlen(ospec)-1]) != -1 )
         && (find_close_char(ospec, 0, OPEN_GROUP, CLOSE_GROUP) == len ))
    ospec = ospec[1..<2];
  return ospec;
}

/**
 * Returns true if an object is either a room or a container.
 *
 * @param  arg the object to test
 * @return     true if arg is valid environment
 */
private int valid_environment(object arg) {
  return (arg->is_room() || arg->is_container());
}

/**
 * Locate the first valid environment in an object's environment path,
 * starting with the object itself.
 *
 * @param  arg the object to test
 * @return     the first room or container containing arg, or 0 if no room
 *             was found
 */
private object find_room(object arg) {
  foreach (object room : ({ arg }) + (all_environment(arg) || ({ }))) {
    if (valid_environment(room)) {
      return room;
    }
  }
  return 0;
}
