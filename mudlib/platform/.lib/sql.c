/**
 * Common functions for SQL clients.
 *
 * @author devo@eotl
 * @alias SqlLib
 */

inherit JSONLib;

string encode_value(mixed value) {
  if (stringp(value)) {
    return sprintf("'%s'", implode(explode(value, "'"), "''"));
  } else if (intp(value)) {
    return to_string(value);
  } else if (floatp(value)) {
    return to_string(value);
  } else if (objectp(value)) {
    return encode_value(ObjectTracker->query_object_id(value));
  } else if (pointerp(value)) {
    return encode_value(json_encode(value));
  } else if (mappingp(value)) {
    return encode_value(json_encode(value));
  } else {
    return "NULL";
  }
}

string get_insert_statement(string table, mapping data, mixed *params) {
  string *columns = m_indices(data);
  params = m_values(data);
  string query;
  if (referencep(params)) {
    query = sprintf("insert into %s (%s) values (%s)", 
                    table, 
                    implode(columns, ","), 
                    implode(map(params, (: "?" :)), ","));
  } else {
    query = sprintf("insert into %s (%s) values (%s)",
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
  if (referencep(params)) {
    string set = "";
    for (int i = 0, j = sizeof(columns); i < j; i++) {
      set = sprintf("%s%s=?,", set, columns[i]);
    }
    set[<1] = 0; // remove trailing comma
    query = sprintf("update %s set %s where %s=?", 
                    table, 
                    set,
                    SQL_ID_COLUMN);
    params += ({ id });
  } else {
    string set = "";
    for (int i = 0, j = sizeof(columns); i < j; i++) {
      set = sprintf("%s%s=%s,", set, columns[i], encode_value(params[i]));
    }
    set[<1] = 0; // remove trailing comma
    query = sprintf("update %s set %s where %s=%s", 
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
  string query;
  if (referencep(params)) {
    string where = "";
    for (int i = 0, j = sizeof(columns); i < j; i++) {
      where = sprintf("%s%s=? and ", where, columns[i]);
    }
    where[<5] = 0; // remove trailing 'and'
    query = sprintf("select * from %s where %s", table, where);
  } else {
    string where = "";
    for (int i = 0, j = sizeof(columns); i < j; i++) {
      where = sprintf("%s%s=%s and ", 
                      where, columns[i], encode_value(params[i]));
    }
    where[<5] = 0; // remove trailing 'and'
    query = sprintf("select * from %s where %s", table, where);
  }
  return query;
}
