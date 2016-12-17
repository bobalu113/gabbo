/**
 * Common functions for SQL clients.
 *
 * @author devo@eotl
 * @alias SqlLib
 */
#include <sys/lpctypes.h>
#include <sql.h>

inherit JSONLib;

string encode_value(mixed value) {
  switch (typeof(value)) {
    case T_NUMBER:
      return to_string(value);
    case T_STRING:             
      return sprintf("'%s'", implode(explode(value, "'"), "''"));
    case T_POINTER:
      return encode_value(json_encode(value));
    case T_OBJECT:
      return encode_value(ObjectTracker->query_object_id(value));
    case T_MAPPING:
      return encode_value(json_encode(value));
    case T_FLOAT:
      return to_string(value);
    case T_CLOSURE:
    case T_SYMBOL:
    case T_QUOTED_ARRAY:
    case T_STRUCT:
    default: 
      return "NULL";
  }
  return 0;
}

string get_insert_statement(string table, mapping data, mixed *params) {
  string *columns = m_indices(data);
  params = m_values(data);
  string query;
  if (referencep(&params)) {
    query = sprintf("insert into %s (%s) values (%s);", 
                    table, 
                    implode(columns, ","), 
                    implode(map(params, (: "?" :)), ","));
  } else {
    query = sprintf("insert into %s (%s) values (%s);",
                    table,
                    implode(columns, ","),
                    implode(map(params, #'encode_value), ",")); //'
  }
  return query;
}

string get_update_statement(string table, mixed id, mapping data, 
                            mixed *params) {
  string *columns = m_indices(data);
  params = m_values(data);
  string query;
  if (referencep(&params)) {
    string set = "";
    for (int i = 0, int j = sizeof(columns); i < j; i++) {
      set = sprintf("%s%s=?,", set, columns[i]);
    }
    set[<1] = 0; // remove trailing comma
    query = sprintf("update %s set %s where %s=?;", 
                    table, 
                    set,
                    SQL_ID_COLUMN);
    params += ({ id });
  } else {
    string set = "";
    for (int i = 0, int j = sizeof(columns); i < j; i++) {
      set = sprintf("%s%s=%s,", set, columns[i], encode_value(params[i]));
    }
    set[<1] = 0; // remove trailing comma
    query = sprintf("update %s set %s where %s=%s;", 
                    table, 
                    set,
                    SQL_ID_COLUMN,
                    encode_value(id));
  }
  return query;
}

string get_select_statement(string table, mapping key, mixed *params) {
  string *columns = m_indices(key);
  params = m_values(key);
  string where = "";
  if (referencep(&params)) {
    for (int i = 0, int j = sizeof(columns); i < j; i++) {
      where = sprintf("%s%s=? and ", where, columns[i]);
    }
    where[<5] = 0; // remove trailing 'and'
  } else {
    for (int i = 0, int j = sizeof(columns); i < j; i++) {
      where = sprintf("%s%s=%s and ", 
                      where, columns[i], encode_value(params[i]));
    }
    where[<5] = 0; // remove trailing 'and'
  }
  string query = sprintf("select * from %s where %s;", table, where);
  return query;
}

string get_create_table_statement(string table, mapping *cols) {
  string columns = "";
  int first = 1;
  foreach (mapping col : cols) {
    string type;
    switch (col[SQL_COL_TYPE]) {
      case SQL_TYPE_INTEGER:  type = "integer"; break;
      case SQL_TYPE_REAL:     type = "real";    break;
      case SQL_TYPE_TEXT:     type = "text";    break;
      case SQL_TYPE_BLOB:     type = "blob";    break;
      case SQL_TYPE_NULL:     
      default:                type = "null";    break;
    }
    string constraint = "";
    if (col[SQL_COL_FLAGS] & SQL_FLAG_PRIMARY_KEY) {
      constraint += "primary key ";
      if (col[SQL_COL_FLAGS] & SQL_FLAG_AUTOINCREMENT) {
        constraint += "autoincrement ";
      }
    } else if (col[SQL_COL_FLAGS] & SQL_FLAG_UNIQUE) {
      constraint += "unique ";
    }
    if (col[SQL_COL_FLAGS] & SQL_FLAG_NOT_NULL) {
      constraint += "not null ";
    }
    if (member(col, SQL_COL_DEFAULT)) {
      constraint += sprintf("default %s ", encode_value(col[SQL_COL_DEFAULT]));
    }
    columns = sprintf("%s%s%s %s %s", 
                      columns, 
                      (first ? "" : ", "),
                      col[SQL_COL_NAME], type, constraint);
    first = 0;
  }
  string query = sprintf("create table %s (\n %s );", table, columns);
  return query;
}

string get_table_info_statement(string table) {
  return sprintf("pragma table_info(%s);", table);
}
