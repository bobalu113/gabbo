inherit CommandCode;

#include <expand_object.h>

#define DEFAULT_CONTEXT        "me"

private variables private functions inherit ArgsLib;
private variables private functions inherit GetoptsLib;
private variables private functions inherit ObjectExpansionLib;

int do_command(string arg) {
  mixed *args;
  args = getopts(explode_args(arg), "");
  if (!sizeof(args[0])) {
    notify_fail("Drop what?\n");
    return 0;
  }

  arg = implode(args[0], " ");
  mixed *targets = expand_objects(arg, THISP, DEFAULT_CONTEXT);

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
      out += sprintf("You must remove %s from %s before you can drop it.\n",
                     target->short(), ENV(target)->short());
    } else if (ENV(target) != THISP) {
      out += sprintf("You aren't holding %s.\n", target->short());
    } else {
      move_object(target, ENV(THISP));
      out += sprintf("You drop %s.\n", target->short());
      room_out += sprintf("%s drops %s.\n", PNAME, target->short());
    }
  }

  write(out);
  tell_room(ENV(THISP), room_out, ({ THISP }));

  return 1;
}

