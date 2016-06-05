/**
 * A module for tracking programs, their clones and blueprints.
 *
 * @author devo@eotl
 * @alias ProgramTracker
 */

inherit SQLTrackerMixin;

// program_id = "program_name#program_time3program_counter"
// ([ program_id : ({ blueprint, ([ clones ]), program_counter }) ])
mapping programs;
// ([ program_name : ({ program_id }) ])
mapping program_names;
int program_counter;

#define PROGRAM_TRACKER   "program"
#define PROGRAM_ID        "program_id"
#define PROGRAM_NAME      "program_name"
#define PROGRAM_TIME      "program_time"
#define PROGRAM_SIZE      "program_size"
#define PROGRAM_BLUEPRINT  0
#define PROGRAM_CLONES     1
#define PROGRAM_COUNT      2

string get_id(string program_name, int program_time, int program_count) {
  return sprintf("%s#%d#%d", program_name, program_time, program_count);
}

string new_blueprint(object blueprint) {
  string program_name = program_name(blueprint);
  string program_time = program_time(blueprint);
  int program_count = ++program_counter;
  string id = get_id(program_name, program_time, program_count);
  mapping pdata = ([ 
    PROGRAM_ID : id,
    PROGRAM_NAME : program_name,
    PROGRAM_TIME : program_time,
    PROGRAM_SIZE : object_info(blueprint, OINFO_MEMORY, OIM_PROG_SIZE)
  ]);

  programs[id] = ({ blueprint, ([ ]), program_count }); 
  if (!member(program_names, program_name)) {
    program_names[program_name] = ({ });
  }
  program_names[program_name] += ({ id });
  SQLTrackerMixin::add_tracked(PROGRAM_TRACKER, pdata);

  return id;
}

string new_clone(object clone) {  
  string id = program_names[program_name(clone)][<1];
  m_add(programs[id][PROGRAM_CLONES], clone); 
  return id;
}

string *query_program_names() {
  return m_indices(program_names);
}

mapping query_program_ids(string program_name) {
  return program_names[program_name];
}

int query_program_count(object ob) {
  string program_name = load_name(ob); // program could have been replaced
  if (!member(program_names, program_name)) {
    return 0;
  }
  return programs[program_names[program_name][<1]][PROGRAM_COUNT];
}

mapping query_clones(string program_id) {
  if (member(programs, program_id)) {
    programs[program_id][PROGRAM_CLONES] -= ([ 0 ]);
    return programs[program_id][PROGRAM_CLONES];
  }
  return 0;
}

int create() {
  SQLTracker::setup_tracker(PROGRAM_TRACKER);
}
