/**
 * The service providing driver hooks.
 *
 * @author devo@eotl
 * @alias HookService
 */
#include <sys/driver_hook.h>
#include <object.h>
#include <sql.h>

inherit ObjectLib;

void telnet_neg_hook(int action, int option, int *opts) {
  return ConnectionTracker->telnet_negotiation(action, option, opts);
}

string auto_include_hook(string base_file, string current_file, int sys) {
  if (current_file && (current_file[<7..] == "/" AutoInclude)) {
    return "";
  } else {
    return "#include <" AutoInclude ">\n";
  }
}

varargs mixed uids_hook(string objectname, object blueprint) {
  // FUTURE implement uids
  return "root";
}

int command_hook(string command, object command_giver) {
  return command_giver->do_command(command);
}

void move_object_hook(object item, object dest) {  
  object origin = ENV(item);
  if (origin && origin->prevent_leave(item, dest)) { return; }
  if (!(item && dest)) { return; }
  if (item->prevent_move(dest)) { return; }
  if (!(item && dest)) { return; }
  if (dest->prevent_enter(item)) { return; }

  set_environment(item, dest);

  if (origin && origin->leave_signal(item, dest)) { return; }
  item->move_signal(origin);
  if (!(item && dest)) { return; }
  dest->enter_signal(item, origin);
  if (!(item && dest)) { return; }
  if (ENV(item) != dest) { return; }

  object oldp = this_player();

  if (living(item)) {
    set_this_player(item);
    dest->init();
    if (ENV(item) != dest) {
      return;
    }
  }

  object *others = all_inventory(dest);

  others[member(others, item)] = 0;
  filter(others, (: 
    if ($2 && living($1)) {
      set_this_player($1);
      $2->init();
    }
    return;
  :), item);

  if (living(item)) {
    set_this_player(item);
    filter_objects(others, "init");
  }

  if (item && living(dest)) {
    set_this_player(dest);
    item->init();
  }

  set_this_player(oldp);
  return;
}

int create_hook(object ob) {
  mixed path_info = get_path_info(ob);
  string zone_id = path_info[PATH_INFO_ZONE];
  
  SqlClientFactory->get_client(DEFAULT_DATABASE);

  if (FINDO(ZoneTracker)) {
    ZoneTracker->new_zone(zone_id);
  }

  if (FINDO(ObjectTracker)) {
    ObjectTracker->new_object(ob);
  }

  int result = ob->create();
  return result;
}

int reset_hook(object ob) {
  return ob->reset();
}

int clean_up_hook(int ref, object ob) {
  return ob->clean_up(ref);
}

private void register_hooks() {
  set_driver_hook(H_TELNET_NEG, unbound_lambda(
    ({ 'action, 'option, 'opts }),
    ({ #'call_other, HookService, "telnet_neg_hook", 'action, 'option, 'opts })
  )); //'

  set_driver_hook(H_AUTO_INCLUDE, unbound_lambda(
    ({ 'base_file, 'current_file, 'sys }),
    ({ #'call_other, HookService, "auto_include_hook", 'base_file, 
                                                       'current_file, 'sys })
  )); //'

  set_driver_hook(H_LOAD_UIDS, unbound_lambda(
    ({ 'objectname }),
    ({ #'call_other, HookService, "uids_hook", 'objectname })
  ));
  set_driver_hook(H_CLONE_UIDS, unbound_lambda(
    ({ 'blueprint, 'objectname }),
    ({ #'call_other, HookService, "uids_hook", 'objectname, 'blueprint })
  )); //'

  set_driver_hook(H_COMMAND, unbound_lambda(
    ({ 'command, 'command_giver }),
    ({ #'call_other, HookService, "command_hook", 'command, 'command_giver })
  )); //'

  set_driver_hook(H_MOVE_OBJECT0, unbound_lambda(
    ({ 'item, 'dest }),
    ({ #'call_other, HookService, "move_object_hook", 'item, 'dest })
  ));

  set_driver_hook(H_CREATE_OB, unbound_lambda(
    ({ 'obj }),
    ({ #'call_other, HookService, "create_hook", 'obj })
  )); //'
  set_driver_hook(H_CREATE_CLONE, unbound_lambda(
    ({ 'obj }),
    ({ #'call_other, HookService, "create_hook", 'obj })
  )); //'
  set_driver_hook(H_CREATE_SUPER, unbound_lambda(
    ({ 'obj }),
    ({ #'call_other, HookService, "create_hook", 'obj })
  )); //'

  set_driver_hook(H_RESET, unbound_lambda(
    0,
    ({ #'call_other, HookService, "reset_hook", ({ #'this_object }) })
  ));

  set_driver_hook(H_CLEAN_UP, unbound_lambda(
    ({ 'ref, 'obj }),
    ({ #'call_other, HookService, "clean_up_hook", 'ref, 'obj })
  ));
}

void create() {
  register_hooks();
}
