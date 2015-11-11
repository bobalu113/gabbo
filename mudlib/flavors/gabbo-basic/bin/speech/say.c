inherit CommandCode;

int do_command(string arg) {
  object env = ENV(THISP);
  if (env->is_room()) {

  } else if (env->is_container()) {
    // check if sound passes through container
  }
}

string get_language(object who) {
  // TODO check user preferences
  // TODO check room/zone
  return "default";
}
