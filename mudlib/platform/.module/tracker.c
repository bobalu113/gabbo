
private mapping trackers;

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
int setup_tracker(string name, mapping config) {
  if (!mappingp(trackers)) {
    trackers = ([ ]);
  }
  if (!mappingp(config)) {
    return 0;
  }
  if (!member(trackers, name)) {
    trackers[name] = config;
  } else {
    foreach (string key, mixed val : config) {
      trackers[name] = val;
    }
  }
  return 1;
}

mixed query_tracker_config(string name, string prop) {
  if (mappingp(trackers[name])) {
    return trackers[name][prop];
  }
  return 0;
}
