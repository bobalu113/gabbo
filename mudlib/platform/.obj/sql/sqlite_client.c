/**
 * 
 * 
 * @author devo@eotl
 * @alias SQLiteClient
 */
inherit SqlLib;

int connect(string file) {
  return sl_open(file);
}

varargs int insert(string table, mapping data, 
                   closure callback, varargs mixed *args) {
  mixed *params;
  string query = get_insert_statement(table, data, &params);
  mixed result = apply(#'sl_exec, query, params); //'
  apply(callback, result, args);
  return 1;
}

varargs int update(string table, mapping data, 
                   closure callback, varargs mixed *args) {
  if (!member(data, SQL_ID_COLUMN)) {
    return 0;
  }
  mixed id = data[SQL_ID_COLUMN];
  data -= ([ SQL_ID_COLUMN ]);

  mixed *params;
  string query = get_update_statement(table, data, &params);
  mixed result = apply(#'sl_exec, query, params); //'
  apply(callback, result, args);
  return 1;
}

varargs int select(string table, mapping key,
                   closure callback, varargs mixed *args) {
  mixed *params;
  string query = get_select_statement(table, key, &params);
  mixed result = apply(#'sl_exec, query, params); //'
  apply(callback, result, args);
  return 1;
}
