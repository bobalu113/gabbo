/**
 * A module for tracking objects/programs as they are loaded and destructed.
 *
 * @author devo@eotl
 * @alias ObjectTracker
 */

inherit SQLTrackerMixin;
inherit MemTrackerMixin;

#define PROGRAM_NAME   "program_name"
#define PROGRAM_TIME   "program_time"
#define OBJECT_NAME    "object_name"
#define OBJECT_TIME    "object_time"
#define DESTRUCT_TIME  "destruct_time"
#define LAST_REF_TIME  "last_ref_time"
#define GIGATICKS      "gigaticks"
#define TICKS          "ticks"

void new_object(object o) {
  mapping pdata = ([ 
    PROGRAM_NAME : program_name(o),
    PROGRAM_TIME : program_time(o)
  ]);
  SQLTrackerMixin::add_tracked(PROGRAM_TRACKER, pdata, (:
    MemTrackerMixin::add_tracked(PROGRAM_TRACKER, $2 + ([ ID_COLUMN : $1 ]));
    mapping odata = ([ 
      OBJECT_NAME : object_name(o),
      OBJECT_TIME : object_time(o),
      PROGRAM : $1
    ]);
    SQLTrackerMixin::add_tracked(OBJECT_TRACKER, odata, (: 
      MemTrackerMixin::add_tracked(OBJECT_TRACKER, $2 + ([ ID_COLUMN : $1 ]));
    :));
  :));
  return;
}

void destruct_object(object o) {
  mapping data = ([
    OBJECT_NAME : object_name(o),
    OBJECT_TIME : object_time(o),
  ]);
  SQLTrackerMixin::query_tracked(OBJECT_TRACKER, odata, (:
    mapping odata = ([ 
      ID_COLUMN : $1[ID_COLUMN],
      DESTRUCT_TIME : time(),
      LAST_REF_TIME : object_info($2, OINFO_BASIC, OIB_TIME_OF_REF),
      GIGATICKS : object_info($2, OINFO_BASIC, OIB_GIGATICKS),
      TICKS :object_info($2, OINFO_BASIC, OIB_TICKS),
    ]);
    SQLTrackerMixin::set_tracked(OBJECT_TRACKER, odata, (:
      MemTrackerMixin::set_tracked(OBJECT_TRACKER, $2);
    :), odata);
  :), o);
  return;
}
