/**
 * A library for manipulating zones of rooms and objects.
 *
 * @author devo@eotl
 * @alias ZoneLib
 */
#include <zone.h>
#include <object.h>

inherit ObjectLib;

struct ZoneInfo {
  string id;
  string parent;
  mapping children;  // ([ zone_id ])
  mapping instances;  // ([ instance_id ])
  string root;
  string flavor;
};

struct ZoneInstanceInfo {
  string id;
  string zone;
  int instance_count;
  string label;
};

int valid_zone_id(string zone_id) {
  return (strstr(zone_id, ZONE_DELIM CATEGORY_DELIM) == -1);
}

string get_parent_zone(string zone_id) {
  if (!valid_zone_id(zone_id)) {
    return 0;
  }
  string *parts = explode(zone_id, ZONE_DELIM);
  if (sizeof(parts) == 1) {
    return 0;
  } else {
    return implode(parts[0..<2], ZONE_DELIM);
  }
}

/**
 * Get the zone an object belongs to.
 *
 * @param  ob the object to query
 * @return    the object's zone
 */
string get_zone(object ob) {
  return get_path_info(ob)[PATH_INFO_ZONE];
}
