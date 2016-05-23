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

// ([ type : ({ mapping *datalist, mapping datamap, string *key, 
//              int min_history, int max_history, 
//              int expire_interval, int next_expire }) ])
private mapping tracked;

varargs void setup_tracker_type(string type, string *key, int min_history, 
                                int max_history, int expire_interval) {  
  if (!member(tracked, type)) {
    tracked[type] = ({ key, ({ }), m_allocate(min_history), 
                       min_history, max_history,
                       expire_interval, time() + expire_interval });
  }
}

mixed *query_tracker_type(string type) {
  if (!member(tracked, type)) {
    return 0;
  }
  return ({ 
    tracked[type][TRACKER_KEY],
    tracked[type][TRACKER_MIN_HISTORY],
    tracked[type][TRACKER_MAX_HISTORY],
    tracked[type][TRACKER_EXPIRE_INTERVAL],
    tracked[type][TRACKER_NEXT_EXPIRE]
  });
}

varargs int add_tracked(string type, mapping data, closure callback, 
                        varargs mixed *args) {
  if (member(data, ID_COLUMN)) {
    return 0;
  }
  if (!member(tracked, type)) {
    setup_tracker_type(type, 0);
  }
  // XXX this could be faster (maybe use linked list)
  tracked[type][TRACKED_DATALIST] += data;  
  mapping datamap = tracked[type][TRACKED_DATAMAP];

  object sql_client = SQLClientFactory->get_client();
  sql_client->insert(TableName(type), data, (:
    $2[ID_COLUMN] = $1;
    $3[$1] = $2;
    apply($4, $2, $5);
  :), data, datamap, callback, args);
  return 1;
}

varargs int set_tracked(string type, mapping data, closure callback,
                        varargs mixed *args) {
  if (!member(tracked, type)) {
    return 0;
  }
  if (member(data, ID_COLUMN)) {
    // if we have an id
    mapping d = tracked[type][TRACKER_DATAMAP][data[ID_COLUMN]];
    foreach (string key, mixed val : data) {
      d[key] = val;
    }
    object sql_client = SQLClientFactory->get_client();
    sql_client->update(TableName(type), 
                       (data - ([ ID_COLUMN ])),
                       ([ ID_COLUMN : data[ID_COLUMN] ]), (:
      apply($2, $1, $3)
    :), d, callback, args);
  } else {
    // lookup id using key
    string *key = tracked[type][TRACKER_KEY];
    if (!sizeof(key & m_indices(data))) {
      return 0;
    }
    object sql_client = SQLClientFactory->get_client();
    closure cb = (:
      if (sizeof($1) == 1) {
        closure cb = (: 
          apply();
        :);
        $2->update($3, 
                   $4,
                   ([ ID_COLUMN : $1[0][0] ]), 
                   cb,
                  );
      } else {
        // TODO error callback
      }
    :);
    sql_client->select(TableName(type), 
                       ({ ID_COLUMN }), 
                       (data & tracked[type][TRACKER_KEY]), 
                       cb)
    sql_client->update(TableName(type), 
                       (data - mkmapping(tracked[type][TRACKER_KEY])),
                       (data & tracked[type][TRACKER_KEY]), (:

      apply($2, $1, $3)
    :), d, callback, args);
  }
  mapping map = tracked[type][TRACKED_TRACKED];
  mixed *history = tracked[type][TRACKED_HISTORY];
  if (!member(map, key)) {
    return 0;
  }
  int size = sizeof(val);
  if (sizeof(map[key]) != size) {
    return 0;
  }
  for (int i = 0; i < size; i++) {
    map[key][i] = val[i];
  }
  return 1;
}

varargs int query_tracked(string type, mixed key, closure callback, 
                          varargs mixed *args) {
  
}

int expire_tracked(string type) {
  if (!member(tracked, type)) {
    return 0;
  }
  mapping map = tracked[type][TRACKED_TRACKED];
  mixed *history = tracked[type][TRACKED_HISTORY];
  int size = sizeof(history);
  if (size > tracked[type][TRACKED_MAX_HISTORY]) {
    mixed *expired = history[0..<(tracked[type][TRACKED_MIN_HISTORY] + 1)];
    history = history[<tracked[type][TRACKED_MIN_HISTORY]..];

  }
}

int get_next_expire() {
  if (!sizeof(tracked)) {
    return 0;
  }
  int next = min(map(m_values(tracked), (: $1[TRACKED_NEXT_EXPIRE] :)));
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
  history = ({ });
  active_map = ([ ]);
}
