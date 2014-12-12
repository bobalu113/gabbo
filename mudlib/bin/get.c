inherit CommandCode;

#include <expand_object.h>

#define DEFAULT_CONTEXT        "here"
#define OPTS                   ""

private variables private functions inherit ArgsLib;
private variables private functions inherit GetoptsLib;
private variables private functions inherit ObjectExpansionLib;

int do_command(string arg) {
  string container;
  mixed *args;
  if (sscanf(arg, "%s from %s", arg, container) == 2) {
    args = getopts(explode_args(arg), OPTS);
    arg = implode(args[0], " ");
    args[0] = sprintf(OPEN_GROUP "%s" CLOSE_GROUP 
                      CONTEXT_DELIM 
                      OPEN_GROUP "%s" CLONE_GROUP, 
                      container, arg);
  } else {
    args = getopts(explode_args(arg), OPTS);
  }

  if (!sizeof(args[0])) {
    notify_fail("Get what?\n");
    return 0;
  }
  mixed *targets = expand_objects(args[0], THISP, DEFAULT_CONTEXT);

  if (!sizeof(targets)) {
    notify_fail("There isn't anything like that here.\n");
    return 0;
  }

  string out = "";
  string room_out = "";
  foreach (mixed *t : targets) {
    object target = t[OB_TARGET];
    string id = t[OB_ID];
    
    if (ENV(target)->is_container()) {
      object *env = all_environment(target);
      int pass = 1;
      object e;
      foreach (e : env) {
        if (e->is_container()) {
          // check all nested containers are open
          if (!e->is_open()) {
            out += sprintf("You must open %s before you can get %s.\n",
                           e->short(), target->short());
            pass = 0;
            break;
          }
        } else {
          // first non-container must be accessible
          // XXX put this test in a lib?
          if (!is_reachable(e)) {
            out += sprintf("You can't get to %s.\n", target->short());
            pass = 0;
          } 
          break;
        }
      }
      if (!pass) {
        continue;
      }
      move_object(target, THISP);
      out += sprintf("You remove %s from %s.\n", 
                     target->short(), ENV(target)->short());
      if (e == ENV(THISP)) {
        room_out += sprintf("%s removes %s from %s.\n",
                            PNAME, target->short(), ENV(target)->short());
      }
    } else if (ENV(target)->is_room()) {
      if (ENV(target) != ENV(THISP)) {
        out += sprintf("You can't get to %s.\n", target->short());
        continue;
      }
      move_object(target, THISP);
      out += sprintf("You pick up %s.\n", target->short());
      room_out += sprintf("%s picks up %s.\n", PNAME, target->short());
    } else if (ENV(target)->is_living()) {
      out += sprintf("You can't get %s from %s, they have to give it to "
                     "you.\n", 
                     target->short(), ENV(target)->query_name());
    }
  }

  write(out);
  tell_room(ENV(THISP), room_out, ({ THISP }));

  return 1;
}

