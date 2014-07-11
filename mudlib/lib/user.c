#include <user.h>

virtual inherit FileLib;

int user_exists(string username) {
  return file_exists(PASSWD_FILE(username));
}

int valid_password(string username, string password) {
  mapping data = restore_value(read_file(PASSWD_FILE(username)));
  return password == data["password"];
}
