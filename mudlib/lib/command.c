inherit FileLib;

string *query_actions() {
  return ({ ({ basename(load_name(THISO)), 0 }) });
}

int do_command(string arg) {
  // FUTURE better fail message and some logging
  notify_fail("Not implemented.");
  return 0;
}
