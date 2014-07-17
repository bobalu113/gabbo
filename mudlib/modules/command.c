/**
 * A module for running user commands.
 * 
 * @author devo@eotl
 * @alias CommandCode
 */

inherit FileLib;

#include <sys/commands.h>

mixed *query_actions() {
  return ({ ({ basename(load_name(THISO)), AA_VERB }) });
}

int do_command(string arg) {
  // FUTURE better fail message and some logging
  notify_fail("Not implemented.");
  return 0;
}
