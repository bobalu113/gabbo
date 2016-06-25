/**
 * A service object for tracking zones and their instances.
 * 
 * @author devo@eotl
 * @alias ZoneTracker
 */

#include <zone.h>

inherit ZoneLib;

// ([ str id : ZoneInfo zone ])
mapping zones;
// ([ str id : ZoneInstance instance ])
mapping instances;
int instance_counter;

struct ZoneInfo new_zone(string zone_id) {
  struct ZoneInfo parent;
  string parent_id = get_parent_zone(zone_id);
  if (parent_id) {
    if (!member(zones[parent_id])) {
      parent = new_zone(parent_id);
    } else {
      parent = zones[parent_id];
    }
  }
  zones[zone_id] = (<ZoneInfo> 
    id: zone_id,
    parent: parent_id,
    children: ([ ]),
    instances: ([ ])
  );
  parent->children += ([ zone_id ]);
  return zones[zone_id];
}

string new_instance(string zone_id, string label) {
  int instance_count = ++instance_counter;
  string instance_id = get_instance_id(zone_id, instance_count);
  instances[instance_id] = (<ZoneInstanceInfo> 
    id: instance_id,
    zone: zone_id,
    instance_count: instance_count,
    label: label
  );
  zones[zone_id]->instances += ([ instance_id ]);
}

string query_starting_instance(string zone_id, string session_id) {
  struct ZoneInfo zone = zones[zone_id];
  if (!zone) {
    zone = new_zone(zone_id);
  }
  if (sizeof(zone->instances)) {
    string instance_id;
    foreach (instance_id : zone->instances) {
      if (instances[instance_id]->label == DEFAULT_INSTANCE) {
        return instance_id;
      }
    }
    return instance_id;
  } else {
    return new_instance(zone_id, DEFAULT_INSTANCE);
  }
}

string get_instance_id(string zone_id, int instance_count) {
  return sprintf("%s#%d", zone_id, instance_count);
}

void create() {
  zones = ([ ]);
  instances = ([ ]);
}
