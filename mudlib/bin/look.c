inherit CommandCode;

private variables private functions inherit ObjectExpansionLib;

// TODO the defaults could have debug info
#define DEFAULT_SHORT          "a nondescript object"
#define DEFAULT_LONG           "It lacks any features whatsoever."
#define DEFAULT_ROOM_SHORT     "An Unnamed Location"
#define DEFAULT_ROOM_LONG      "You see nothing of interest."
#define MULTI_EXIT_MSG         "Obvious exits are: "
#define SINGLE_EXIT_MSG        "The only obvious exit is "
#define NO_EXIT_MSG            "There are no obvious exits."
#define VOID_MSG               "You are floating in a formless void."

#define DEFAULT_CONTEXT        "here"

int do_command(string arg) {
  // TODO add max args param to explode_args
  // TODO opt to not update context
  mixed *args = getopts(explode_args(arg), "hm");

  // scope to room, include detail ids
  mixed *targets = expand_objects(args[0], 
                                  THISP,
                                  DEFAULT_CONTEXT, 
                                  EXPAND_DETAIL|UPDATE_CONTEXT);

  if (!sizeof(targets)) {
    if (!environment(THISP)) {
      write(VOID_MSG "\n");
      return 1;
    } else {
      notify_fail("There isn't anything like that here.\n");
      return 0;
    }
  }

  string out = "";
  foreach (mixed *t : targets) {
    object target = t[OB_TARGET];
    string id = t[OB_ID];
    string detail = t[OB_DETAIL];

    if (member(opts, 'h')) {
      // TODO this should be configurable (id, short, silent, etc)
      out += sprintf("::::: %s :::::", file_name(target));
    }
    if (detail) {
      out += format_detail(target, THISP, id, detail);
    } else {
      if (target->is_room()) {
        // TODO add brief support
        out += format_room(target, THISP, id);
      } else {
        out += format_default(target, THISP, id);
      }
    }
  }

  if (member(opts, 'm')) {
    // TODO add pager support
    write(out);
  } else {
    write(out);
  }

  return 1;
}

string format_room(object target, object looker, string id) {
  string short = target->query_short(looker);
  string long = target->query_long(looker, id);
  mapping exits = target->query_exits();
  string *dirs;
  if (mappingp(exits)) {
    dirs = m_indices(filter(exits, (: $2[1] :)));
    dirs = sort_array(dirs, #'>);
  } else {
    dirs = ({ ]});
  }

  int width = looker->query_page_width();
  int exit_count = sizeof(dirs);
  return sprintf("%s%-=*s---- %-=*s\n%s", 
    (stringp(short) ? short + "\n" : DEFAULT_ROOM_SHORT),
    width,
    (stringp(long) ? sprintf("    %s\n\n", long) : DEFAULT_ROOM_LONG),
    width,
    ((exit_count > 1 ) ? 
      sprintf(MULTI_EXIT_MSG "%s.", implode(dirs, ", ")) :
      (exit_count == 1) ?
      sprintf(SINGLE_EXIT_MSG "%s.", dirs[0]) :
      NO_EXIT_MSG
    ),
    format_inventory(target, looker)
    );
}

string format_default(object target, object looker, string id) {
  int width = looker->query_page_width();
  string long = target->query_long(looker, id);
  return sprintf("%-=*s\n", 
    width, 
    (stringp(long) ? long : DEFAULT_LONG)
  );
}

string format_detail(object target, object looker, string id, string detail) {
  int width = looker->query_page_width();
  string description = target->query_detail(detail);
  return sprintf("%-=*s\n", width, description);
  );
}

string format_inventory(object target, object looker) {
  string *items = map(all_inventory(target), #'format_item, looker);
  return implode(items, "\n") + "\n";
}

string format_item(object target, object looker) {
  string short = target->query_short(looker);
  return (stringp(short) ? short : DEFAULT_SHORT);
}
