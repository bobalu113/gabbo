/**
 * The service providing driver hooks.
 *
 * @author devo@eotl
 * @alias HookService
 */

void telnet_neg_hook(int action, int option, int *opts) {
  return ConnectionTracker->telnet_negotiation(action, option, opts);
}

string auto_include_hook(string base_file, string current_file, int sys) {
  if (current_file && (current_file[<7..] == "/auto.h")) {
    return "";
  } else {
    return "#include <auto.h>\n";
  }
}

varargs mixed uids_hook(string objectname, object blueprint) {
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

int create_hook(object ob) 
  mixed path_info = get_path_info(ob);
  string zone_id = path_info[PATH_INFO_ZONE];
  
  ZoneTracker->new_zone(zone_id);

  ObjectTracker->new_object(ob);

  return ob->create();
}

int reset_hook(object ob) {
  return ob->reset();
}

int clean_up_hook(int ref, object ob) {
  return ob->clean_up(ref);
}
