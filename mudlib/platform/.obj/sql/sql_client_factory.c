/** 
 *
 * @author devo@eotl
 * @alias SqlClientFactory
 */

#include <sql.h>

// ([ str file : obj client ])
mapping sqlite_clients;

object get_sqlite_client(string file) {
  if (member(sqlite_clients, file) && sqlite_clients[file]) {
    return sqlite_clients[file];
  }
  object client = clone_object(SQLiteClient);
  if (client->connect(file)) {
    return client;
  }
  destruct(client);
  return 0;
}

object get_client(string database) {
  int pos = strstr(database, ":");
  string type = database[0..(pos - 1)];
  database = database[(pos + 1)..];
  switch (type) {
    case "sqlite":
      return get_sqlite_client(database);
      break;
    default:
      raise_error("Unknown database type: " + type);
      break;
  }
  return 0;
}

int create() {
  load_object(SQLiteClient);
  sqlite_clients = ([ ]);
  return 0;
}