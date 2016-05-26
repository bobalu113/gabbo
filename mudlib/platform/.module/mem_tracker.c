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

private mapping tracked;

private mapping next_expire;

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
int setup_mem_tracker(string name, closure keymaker, varargs int *args) {
  int size = sizeof(args);
  int min_size = ((size >= 1) ? args[0] : DEFAULT_MIN_SIZE);
  int max_size = ((size >= 1) ? args[0] : DEFAULT_MAX_SIZE);
  int expire_interval = ((size >= 1) ? args[0] : DEFAULT_EXPIRE_INTERVAL);

  int result = TrackerMixin::setup_tracker(name, ([ 
    TRACKER_KEYMAKER : keymaker,
    TRACKER_MIN_SIZE : min_size,
    TRACKER_MAX_SIZE : max_size,
    TRACKER_EXPIRE_INTERVAL : expire_interval
  ]));

  expire_tracked(name);
  return result;
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
varargs int add_tracked(string name, mapping data, closure callback, 
                        varargs mixed *args) {
  if (member(data, ID_COLUMN)) {
    return 0;
  }
  if (!member(tracked, name)) {
    return 0;
  }

  closure keymaker = get_tracker_config(name, TRACKER_KEYMAKER);
  mixed key = funcall(keymaker, data);

  data[ID_COLUMN] = key; 
  tracked[name][TRACKED_MAP] = data;
  // XXX this could be faster (maybe use linked list of some sort)
  tracked[name][TRACKED_LIST] += data;
  apply(callback, data, args); 

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
varargs int set_tracked(string name, mapping data, closure callback,
                        varargs mixed *args) {
  if (!member(tracked, name)) {
    return 0;
  }
  if (!member(data, ID_COLUMN)) {
    return 0;
  }

  tracked[name][TRACKED_MAP] = data;
  mapping d = tracked[name][TRACKED_MAP][data[ID_COLUMN]];
  foreach (string key, mixed val : data) {
    d[key] = val;
  }
  apply(callback, d, args); 
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
varargs int query_tracked(string type, mixed id, closure callback, 
                          varargs mixed *args) {
  if (!member(tracked, name)) {
    return 0;
  }

  apply(callback, tracked[name][TRACKED_MAP][id], args); 
  return 1;
}

/**
 * Expire stale records for the specified tracker type. If the record count
 * exceeds the maximum, all records in exceeds of the mimimum will be freed
 * from memory.
 * 
 * @param  type the tracker type
 * @return      the number of expired records
 */
int expire_tracked(string type) {
  if (!member(tracked, type)) {
    return 0;
  }
  mapping map = tracked[type][TRACKED_MAP];
  mixed *list = tracked[type][TRACKED_LIST];
  int size = sizeof(list);
  if (size > tracked[type][TRACKED_MAX_SIZE]) {
    mixed *expired = list[0..<(tracked[type][TRACKER_MIN_SIZE] + 1)];
    list = list[<tracked[type][TRACKER_MIN_SIZE]..];
    foreaach (mapping )
  }
}

/**
 * Get the time of the next expiration attempt, based on the next expiration
 * times of all tracked types. If no expiration intervals have been set,
 * the default reset time will be used.
 * 
 * @return the number of seconds until the next expiration attempt
 */
int get_next_expire() {
  if (!sizeof(tracked)) {
    return 0;
  }
  int next = min(map(m_values(tracked), (: $1[TRACKER_NEXT_EXPIRE] :)));
  return max(0, (next - time()));
}

int reset() {
  int time = time();
  foreach (string type, mixed *t : tracked) {
    if (t[TRACKED_NEXT_EXPIRE] < time) {
      expire_tracked(type);
      t[TRACKED_NEXT_EXPIRE] = time + t[TRACKED_EXPIRE_INTERVAL];
    }
  }
  return get_next_expire();
}

void setup_tracker() {
  tracked = ([ ]);
}
