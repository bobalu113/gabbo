/**
 * A module for tracking objects as they are loaded and destructed.
 *
 * @author devo@eotl
 * @alias ObjectTracker
 */

inherit SQLTrackerMixin;

#define OBJECT_TRACKER  "object"
#define OBJECT_ID       "object_id"
#define OBJECT_NAME     "object_name"
#define OBJECT_TIME     "object_time"
#define DESTRUCT_TIME   "destruct_time"
#define LAST_REF_TIME   "last_ref_time"
#define GIGATICKS       "gigaticks"
#define TICKS           "ticks"

void created_signal(object o) {
  string program_id;
  if (!clonep(o)) {
    program_id = ProgramTracker->new_blueprint(o);
  } else {
    program_id = ProgramTracker->new_clone(o);
  }
  mapping odata = ([ 
    OBJECT_ID : get_id($2),
    OBJECT_NAME : object_name($2),
    OBJECT_TIME : object_time($2),
    PROGRAM : program_id
  ]);
  SQLTrackerMixin::add_tracked(OBJECT_TRACKER, odata);
  return;
}

void destructed_signal(object o) {
  mapping odata = ([ 
    ID_COLUMN : get_id(o),
    DESTRUCT_TIME : time(),
    LAST_REF_TIME : object_info($2, OINFO_BASIC, OIB_TIME_OF_REF),
    GIGATICKS : object_info($2, OINFO_BASIC, OIB_GIGATICKS),
    TICKS : object_info($2, OINFO_BASIC, OIB_TICKS),
  ]);
  SQLTrackerMixin::set_tracked(OBJECT_TRACKER, odata);
  return;
}

// Reminder: clone object names are "load_name#clone_counter"
// blueprint_object_id = "object_name#object_time#blueprint_counter"
// clone_object_id = "object_name#object_time"
string get_id(object ob) {
  if (clonep(ob)) {
    return sprintf("%s#%d", object_name(ob), object_time(ob));
  } else {
    return sprintf("%s#%d#%d", 
                   object_name(ob), object_time(ob), 
                   ProgramTracker->query_program_count(ob));
  }
}

int create() {
  SQLTracker::setup_tracker(OBJECT_TRACKER);
}