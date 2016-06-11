/**
 *
 * @author devo@eotl
 * @alias SqlMixin
 */
#include <sql.h>

// TODO expand interface to support objects that use more than one db
string database;

void setup_sql() {
  database = DEFAULT_DATABASE;
  return;
}

int set_database(string db) {
  database = db;
  return 1;
}

string query_database() {
  return database;
}

/**
 * Insert a new record of tracked data. Data must be in mapping form. If the
 * tracked type doesn't yet exist, will be created with a default 
 * configuration. First, the data is added to the in-memory record list without
 * an id. Next, the data will be inserted into the backing SQL database. The
 * auto-generated id will then be added to the in-memory record map. A callback 
 * function will be invoked upon successful completion of the update operation. 
 * This may be synchronous or asynchronous depending on the SQL driver being 
 * used.
 * 
 * @param  type     the tracker type
 * @param  data     the tracked data
 * @param  callback a closure to call once tracking is active 
 * @param  args     extra args to pass to the callback
 * @return          0 for failure, 1 for success
 */
protected varargs int insert(string table, mapping data, 
                             closure callback, varargs mixed *args) {
  object sql_client = SqlClientFactory->get_client(database);
  sql_client->insert(
    table, 
    data, 
    (:
      // TODO add id to data in order to support sequences
      // sequence support needs some validation changes elsewhere in the code
      // $2[SQL_ID_COLUMN] = $1; 
      return apply($3, $2, $4); // call callback with data
    :), 
    data,
    callback, 
    args
  );
  return 1;
}

/**
 * Update an existing record of tracked data. Data must be in mapping form, and
 * must either contain a valid id or all keys represented in the keys array
 * for the configured tracker type. Either the id or keys may be provided, and
 * all other members of the passed data will be the new values to sign to the
 * other members of the existing tracked data. If both the id and keys are
 * provided, the id will be used to identify the record, and the key values
 * will treated as any other value to update (provided the maintain 
 * uniqueness). It is not possible to update the id. A callback function will
 * be invoked upon successful completion of the update operation. This may be
 * synchronous or asynchronous depending on the SQL driver being used.
 * 
 * @param  type     the tracker type
 * @param  data     the tracked data
 * @param  callback a closure to call when the update is complete
 * @param  args     extra args to pass to the callback
 * @return          0 for failure, 1 for success
 */
protected varargs int update(string table, mapping data, 
                             closure callback, varargs mixed *args) {
  if (!member(data, SQL_ID_COLUMN)) {
    return 0;
  }
  object sql_client = SqlClientFactory->get_client(database);
  sql_client->update(
    table, 
    (data - ([ SQL_ID_COLUMN ])),
    ([ SQL_ID_COLUMN : data[SQL_ID_COLUMN] ]), 
    (:
      apply($2, $1, $3)
    :), 
    callback, 
    args
  );
  return 1;
}

/**
 * Look up a tracked data record using a specified key. The key may be 
 * expressed as a mapping, in which case it must contain either the id
 * or the tracker type's keys. It may also be passed as an integer, to be used
 * as the id. If found, the record will be passed to the provided callback
 * closure.
 * 
 * @param  type     the tracked type
 * @param  key      a mapping of data to be used as a key
 * @param  callback a closure to call with the selected record
 * @param  args     extra args to pass to the callback
 * @return          0 for failure, 1 for success
 */
protected varargs int select(string table, mapping key, 
                             closure callback, varargs mixed *args) {
  object sql_client = SqlClientFactory->get_client(database);
  sql_client->select(
    table, 
    key,
    (:
      return apply($2, $1, $3);
    :), 
    callback, 
    args
  );
  return 1;
}

