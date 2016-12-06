/**
 * Some common validation functions for commands.
 *
 * @author devo@eotl
 * @alias ValidationLib
 */

inherit UserLib;

int validate_max_length(string arg, mixed len) {
  return (strlen(arg) <= to_int(len));
}

int validate_min_length(string arg, mixed len) {
  return (strlen(arg) >= to_int(len));
}

int validate_not_empty(string arg) {
  return validate_min_length(arg, 1);
}

int validate_is_user(string username) {
  return user_exists(username);
}

