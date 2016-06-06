/**
 * A module for tracking programs, their clones and blueprints.
 *
 * @author devo@eotl
 * @alias ProgramTracker
 */

inherit SQLMixin;
inherit ProgramLib;

// program_id = "program_name#program_time3program_count"
// ([ str program_id : ProgramInfo info ])
private mapping programs;
// ([ str program_name : ({ str program_id }) ])
private mapping program_names;
// ([ obj ob : str program_id ])
private mapping object_map;
private int program_counter;

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

public string new_program(object blueprint) {
  string program_name = program_name(blueprint);
  string program_time = program_time(blueprint);
  int program_count = ++program_counter;
  string id = get_id(program_name, program_time, program_count);

  programs[id] = (<ProgramInfo> 
    id: id,
    blueprint: blueprint,
    clones: ([ ]),
    program_count: program_count
  ); 
  if (!member(program_names, program_name)) {
    program_names[program_name] = ({ });
  }
  program_names[program_name] += ({ id });
  object_map[blueprint] = id;

  mapping pdata = ([ 
    PROGRAM_ID : id,
    PROGRAM_NAME : program_name,
    PROGRAM_TIME : program_time,
    PROGRAM_SIZE : object_info(blueprint, OINFO_MEMORY, OIM_PROG_SIZE)
  ]);
  SQLMixin::insert(PROGRAM_TABLE, pdata);

  return id;
}

public string program_cloned(object clone) {  
  string id = program_names[program_name(clone)][<1];
  m_add(programs[id]->clones, clone); 
  object_map[clone] = id;
  return id;
}

public string *query_program_names() {
  return m_indices(program_names);
}

public mapping query_program_ids(string program_name) {
  return program_names[program_name];
}

public string query_program_id(object ob) {
  if (!ob) {
    return 0;
  }
  return object_map[ob];
}

public int query_program_count(string id) {
  if (!member(programs, id)) {
    return 0;
  }
  return programs[id]->program_count;
}

public mapping query_clones(string program_id) {
  if (member(programs, program_id)) {
    programs[program_id]->clones -= ([ 0 ]);
    return programs[program_id]->clones;
  }
  return 0;
}

protected int setup() {
  SQLTracker::setup_sql();
  // TODO fill mem cache with existing programs/clones from objdump
}

int create() {
  return setup();
}

