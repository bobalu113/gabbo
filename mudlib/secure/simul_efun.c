/**
 * The simulated efun object.
 *
 * @author devo@eotl
 * @alias SimulEfunObject
 */

#include <sys/functionlist.h>
#include <capabilities.h>

// ([ username : active_avatar ])
mapping players;

object find_player(string username) {
  //return players[username];
  object *u = filter(users(), (: $1->query_username() == $2 :), username);
  if (sizeof(u)) {
    return u[0];
  } else {
    return 0;
  }
}

int move_resolved(mixed item, mixed dest) {
  move_object(item, dest);
  if (!objectp(item)) { item = find_object(item); }
  if (!objectp(dest)) { dest = find_object(dest); }
  return (item && dest && (environment(item) == dest));
}

mapping query_capabilities(object ob) {
  mapping result = ([ ]);
  mixed *vars = variable_list(ob, RETURN_FUNCTION_NAME
                                  | RETURN_VARIABLE_VALUE);
  int i = 0;
  while ((i = member(vars, CAPABILITIES_VAR_STR, i)) != -1) {
    mixed val = vars[++i];
    if (mappingp(val)) {
      result += val;
    }
    i++;
  }
  return result;
}

varargs int send_binary_message(object who, mixed message, int flags) {
  object old = previous_object();
  set_this_object(who);
  int result = binary_message(message, flags);
  set_this_object(old);
  return result;
}

nomask void write(varargs mixed *args) {
  raise_error("write() efun is deprecated, use MessageLib");
}

nomask void tell_room(varargs mixed *args) {
  raise_error("tell_room() efun is deprecated, use MessageLib");
}

nomask void say(varargs mixed *args) {
  raise_error("say() efun is deprecated, use MessageLib");
}

void tell_object(varargs mixed *args) {
  raise_error("tell_object() efun is deprecated, use MessageLib");
}

nomask void printf(varargs mixed *args) {
  raise_error("printf() efun is deprecated, use MessageLib");
}
