/**
 * A module for tracker objects. The tracked metadata will be maintained in an 
 * ordered list (by time of creation) and a map using auto-generated keys,
 * which will be returned upon insertion. Each tracked type must be backed by
 * a SQL table with an auto-incrementing column, which will also contain any
 * tracked metadata. The metadata will be passed as a mapping, with the keys
 * denoting column names in the backing table. After insertion, an 'id' element
 * will be added with the id.
 *
 * @author devo@eotl
 * @alias TrackerMixin
 */

/**
 * Configure a tracker type. If the type already exists, it will be 
 * re-configured and records over the maximum will be expired from memory.
 * 
 * @param type            the tracker type
 * @param key             a array of keys from your tracked data; the values 
 *                        for those keys will be used to uniquely identify a 
 *                        record that hasn't yet been given an id
 * @param min_history     the minimum number of records that will be maintained
 *                        in the in-memory tracked map
 * @param max_history     the maximum number of records that will be maintained
 *                        in the in-memory tracked map
 * @param expire_interval the number of seconds to wait before attempting to
 *                        expire records from the in-memory tracked map
 * @return                the next expiration time
 */
int setup_tracker(string type) {
  return TrackerMixin::setup_tracker(type, ([ ]));
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
varargs int add_tracked(string type, mapping data, closure callback, 
                        varargs mixed *args) {
  if (member(data, ID_COLUMN)) {
    return 0;
  }

  object sql_client = SQLClientFactory->get_client();
  sql_client->insert(
    TableName(type), 
    data, 
    (:
      $2[ID_COLUMN] = $1; // add id to data
      apply($3, $2, $4); // call callback with data
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
int set_tracked(string type, mapping data, closure callback,
                varargs mixed *args) {
  if (!member(data, ID_COLUMN)) {
    return 0;
  }
  object sql_client = SQLClientFactory->get_client();
  sql_client->update(
    TableName(type), 
    (data - ([ ID_COLUMN ])),
    ([ ID_COLUMN : data[ID_COLUMN] ]), 
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
int query_tracked(string type, mapping data, closure callback,
                  varargs mixed *args) {
  object sql_client = SQLClientFactory->get_client();
  sql_client->select(
    TableName(type), 
    data,
    (:
      apply($2, $1, $3)
    :), 
    callback, 
    args
  );
  return 1;
}

