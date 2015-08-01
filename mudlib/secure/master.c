/**
 * The master object.
 *
 * @author devo@eotl
 * @alias MasterObject
 */

/*  MASTER_NAME      (#define in config.h, or as commandline option)
**  secure/master.c  (native default)
**
** The master is the gateway between the gamedriver and the mudlib to perform
** actions with mudlib specific effects.
** Calls to the master by the gamedriver have an automatic catch() in effect.
**
** Note that the master is loaded first of all objects. Thus it is possible,
** you shouldn't inherit an other object (as most files expect the master
** to exist), nor is the compiler able to search include files
** (read: they must be specified with full path).
*/

#include "/include/auto.h"
#include "/include/sys/driver_hook.h"
#include "/include/sys/debug_info.h"

//===========================================================================
//  Initialisation
//
// These functions are called after (re)loading the master to establish the
// most basic operation parameters.
//
// The initialisation of LPMud on startup follows this schedule:
//   - The gamedriver evaluates the commandline options and initializes
//     itself.
//   - The master is loaded, but since the driverhooks are not set yet,
//     no standard initialisation lfun is called.
//   - get_master_uid() is called. If the result is valid, it becomes the
//     masters uid and euid.
//   - inaugurate_master() is called.
//   - flag() is called for each given '-f' commandline option.
//   - get_simul_efun() is called.
//   - the WIZLIST is read in.
//   - epilog() is called. If it returns an array of strings, they are given
//     one at a time as argument to preload().
//     Traditionally, these strings are the filenames of the objects to
//     preload, which preload() then does.
//   - The gamedriver sets up the IP communication and enters the backend
//     loop.
//
// If the master is reloaded during the game, this actions are taken:
//   - The master is loaded, and its initialisation lfun is called according
//     to the settings of the driverhooks (if set).
//   - Any auto-include string and all driverhooks are cleared.
//   - get_master_uid() is called. If the result is valid, it becomes the
//     masters uid and euid.
//   - inaugurate_master() is called.
//
// If the master was destructed, but couldn't be reloaded, the old
// master object could be reactivated. In that case:
//   - reactivate_destructed_master() is called.
//   - inaugurate_master() is called.
//===========================================================================

void inaugurate_master(int arg) {
  // XXX what is this?
   if (!arg) {
    set_extra_wizinfo(0, 5);
  }

  // return dirs with trailing backslash
  // FUTURE add support for local include dirs
  set_driver_hook(H_INCLUDE_DIRS, ({ "/include/" }));

  // FUTURE add support for local auto includes
  set_driver_hook(H_AUTO_INCLUDE, unbound_lambda(
    ({ 'base_file, 'current_file, 'sys }),
    ({ #'?,
       ({ #'==, 'current_file, "/include/auto.h" }),
       "",
       "#include <auto.h>\n"
    })
  ));

  // FUTURE implement uids
  set_driver_hook(H_LOAD_UIDS, unbound_lambda(
    ({ 'objectname }),
    ({ #'return, "root" })
  ));

  set_driver_hook(H_CLONE_UIDS, unbound_lambda(
    ({ 'blueprint, 'objectname }),
    ({ #'return, "root" })
  ));

  // FUTURE allow customization
  set_driver_hook(H_NOTIFY_FAIL, unbound_lambda(
    ({ 'command, 'cmd_giver }),
    ({ #'return, "Huh?\n" })
  ));

  set_driver_hook(H_MOVE_OBJECT0, unbound_lambda(
    ({'item, 'dest }),
    ({ #'?,
       ({ #',,
          ({ #'=, 'origin, ({ #'environment, 'item }) }),
          0
       }),
       0, // no op
       ({ #'?,
          // check prevent_leave
          ({ #'&&,
             'origin,
             ({ #'call_other,
                'origin,
                "prevent_leave",
                'item,
                'dest
             }),
          }),
          1, // fail prevent_leave
          ({ #'!, 'item }),
          1,
          ({ #'!, 'dest }),
          1,
          // check prevent_move
          ({ #'call_other,
             'item,
             "prevent_move",
             'dest
          }),
          1,
          ({ #'!, 'item }),
          1,
          ({ #'!, 'dest }),
          1,
          // check prevent_enter
          ({ #'call_other,
             'dest,
             "prevent_enter",
             'item
          }),
          1, // fail prevent_enter
          ({ #'!, 'item }),
          1,
          ({ #'!, 'dest }),
          1,
          0 // all pass
       }),
       0, // prevent failed
       // move
       ({ #',, ({ #'efun::set_environment, 'item, 'dest }), 0 }),
       0, // no op
       // signal leave
       ({ #'&&,
          'origin,
          ({ #'call_other,
             'origin,
             "leave_signal",
             'item,
             'dest
          }),
          0
       }),
       0, // no op
       ({ #'!, 'item }),
       0,
       ({ #'!, 'dest }),
       0,
       // signal move
       ({ #',,
          ({ #'call_other,
            'item,
            "move_signal",
            'origin
          }),
          0
       }),
       0, // no op
       ({ #'!, 'item }),
       0,
       ({ #'!, 'dest }),
       0,
       // signal enter
       ({ #',,
          ({ #'call_other,
             'dest,
             "enter_signal",
             'origin
          }),
          0
       }),
       0, // no op
       ({ #'!, 'item }),
       0,
       ({ #'!, 'dest }),
       0,
       // sanity check
       ({ #'==, ({ #'environment, 'item }), 'dest }),
       ({ #',,
          // dest->init(item)
          ({ #'?,
             ({ #'living, 'item }),
             ({ #',,
                ({ #'efun::set_this_player, 'item }),
                ({ #'call_other, 'dest, "init" }),
                ({ #'?,
                   ({ #'!=, ({ #'environment, 'item }), 'dest }),
                   ({ #'return })
                })
             })
          }),
          // item->init(inv(dest))
          ({ #'=, 'others, ({ #'all_inventory, 'dest }) }),
          ({ #'=, ({ #'[, 'others, ({ #'member, 'others, 'item }) }), 0 }),
          ({ #'filter,
             'others,
             ({ #'bind_lambda, unbound_lambda(
                ({ 'ob, 'item }),
                ({ #'?,
                   ({ #'living, 'ob }),
                   ({ #',,
                      ({ #'efun::set_this_player, 'ob }),
                      ({ #'call_other, 'item, "init" })
                   })
                })
             ) }),
            'item
          }),
          // inv(dest)->init(item)
          ({ #'?,
             ({ #'living, 'item }),
             ({ #',,
                ({ #'efun::set_this_player, 'item }),
                ({ #'filter_objects, 'others, "init" }),
             })
          }),
          // item->init(dest)
          ({ #'?,
             ({ #'living, 'dest }),
             ({ #',,
                ({ #'efun::set_this_player, 'dest }),
                ({ #'call_other, 'item, "init" }),
             })
          })
       })
    })
  ) );

  set_driver_hook(H_CREATE_OB, unbound_lambda(({ 'obj }),
    ({ #'call_other, 'obj, "create" })
  ));
  set_driver_hook(H_CREATE_CLONE, unbound_lambda(({ 'obj }),
    ({ #'call_other, 'obj, "create" })
  ));
  set_driver_hook(H_CREATE_SUPER, unbound_lambda(({ 'obj }),
    ({ #'call_other, 'obj, "create" })
  ));

  set_driver_hook(H_RESET, unbound_lambda(
    0,
    ({ #'call_other, ({ #'this_object }), "reset" })
  ));
  set_driver_hook(H_CLEAN_UP, unbound_lambda(
    ({ 'ref, 'obj }),
    ({ #'call_other, 'obj, "clean_up" }) // '
  ));
}

string get_master_uid() {
  return "root";
}

void flag(string arg) {
  string obj, fun, rest;

  if (arg == "shutdown") {
    shutdown();
    return;
  }
  write("master: Unknown flag " + arg +  "\n");
}

string *epilog(int eflag) {
  return ({ });
}

void preload(string file) {
  catch (load_object(file));
}

string get_simul_efun() {
  load_object(SimulEfunObject);
  return SimulEfunObject;
}

object connect() {
  return clone_object(LoginObject);
}

void disconnect(object obj, string remaining) {
  // FUTURE implement notifier
  // TODO echo disconnect to room
}

void remove_player(object player) {
  // TODO log player out
}

void stale_erq(closure callback) {
  // TODO report failure appropriately
}


//===========================================================================
//  Runtime Support
//
// Various functions used to implement advanced runtime features.
//===========================================================================

object compile_object(string filename) {
  // XXX implement a virtual object?
  return 0;
}

mixed include_file(string file, string compiled_file, int sys_include) {
  return 0;
}

mixed inherit_file(string file, string compiled_file) {
  return 0;
}

string get_wiz_name(string file) {
  // FUTURE implement wizlist
  return "root";
}

mixed prepare_destruct(object obj) {
  // TODO implement destruct_signal
  return 0;
}

void receive_udp(string host, string msg, int port) {
  // FUTURE impelement intermud
}

void slow_shut_down(int minutes) {
  // FUTURE implement friendly shutdown
}

varargs void notify_shutdown(string crash_reason) {
  // TODO implement
}


//===========================================================================
//  Error Handling
//
//===========================================================================

// TODO rewrite these to use new Logger API (maybe)

private string format_stacktrace(string curobj, int line, int caught,
                                 mixed *debug_info);

void dangling_lfun_closure() {
  raise_error("dangling lfun closure\n");
}

void log_error(string file, string err, int warn) {
  string msg = sprintf("[%s] %s:%s", (warn ? "WARNING" : "ERROR"), file,
    err);
  write_file("/log/compile.log", msg);
  write(msg);
}

mixed heart_beat_error(object culprit, string err, string prg, string curobj,
                       int line, int caught) {
  string msg = sprintf("%s,%d %-5s %s %s - %s",
    strftime("%Y-%m-%d %H:%M:%S"), utime()[1], "ERROR", to_string(culprit),
    prg, err);

  write_file("/log/heartbeat.log", msg);
  write(msg);
  return 0;
}

void runtime_error(string err, string prg, string curobj, int line,
                   mixed culprit, int caught) {
  string msg = sprintf("%s,%d %-5s %s %s - %s%s",
    strftime("%Y-%m-%d %H:%M:%S"),
    utime()[1],
    "ERROR",
    to_string(culprit),
    to_string(prg),
    err,
    format_stacktrace(curobj, line, caught, debug_info(7, 1))
  );

  write_file("/log/runtime.log", msg);
  write(msg);
}

void runtime_warning(string err, string curobj, string prg, int line,
                     int inside_catch) {
  string msg = sprintf("%s,%d %-5s %s - %s%s",
    strftime("%Y-%m-%d %H:%M:%S"),
    utime()[1],
    "WARN",
    to_string(prg),
    err,
    format_stacktrace(curobj, line, inside_catch, debug_info(7, 1))
  );

  write_file("/log/runtime.log", msg);
  //write(msg);
}

private string format_stacktrace(string curobj, int line, int caught,
                                 mixed *debug_info) {
  string result = "";

  if (caught) {
    result += sprintf("Caught by: %s:%d\n", curobj, line);
  }

  if (sizeof(debug_info) > 1) {
    mixed *stack = debug_info[1..];
    for (int i = sizeof(stack) - 1; i >= 0; i--) {
      result += sprintf("%10s %s->%s(%s:%d)\n", "at", stack[i][TRACE_OBJECT],
        to_string(stack[i][TRACE_NAME]), stack[i][TRACE_PROGRAM],
        stack[i][TRACE_LOC]);
    }
  }

  if (sizeof(debug_info) && debug_info[0]) {
    result += sprintf("Caused by: %s\n", debug_info[0]);
  }

  return result;
}

//===========================================================================
//  Security and Permissions
//
// Most of these functions guard critical efuns. A good approach to deal
// with them is to redefine the efuns by simul_efuns (which can then avoid
// trouble prematurely) and give root objects only the permission to
// execute the real efuns.
//
// See also valid_read() and valid_write().
//===========================================================================

int privilege_violation(string op, mixed who, mixed arg, mixed arg2) {
  switch (op) {

    case "input_to":
      if (load_name(who) == LoginObject) {
        return 1;
      }
      break;

    default:
      return 1;
  }

  return 1;
}

int query_allow_shadow(object victim) {
  // TODO implement
  return 1;
}

int valid_trace(string what, mixed arg) {
  // TODO implement
  return 1;
}

int valid_exec(string name, object ob, object obfrom) {
  // TODO implement
  return 1;
}

int valid_query_snoop(object obj) {
  // TODO implement
  return 1;
}

int valid_snoop(object snoopee, object snooper) {
  // TODO implement
  return 1;
}


//===========================================================================
//  Userids and depending Security
//
// For each object in the mud exists a string attribute which determines the
// objects rights in security-sensitive matters. In compat muds this attribute
// is called the "creator" of the object, in !compat muds the object's "userid"
// ("uid" for short).
//
// "Effective Userids" are an extension of this system, to allow the easier
// implementation of mudlib security by diffentiating between an objects
// theoretical permissions (uid) and its current permissions (euid) (some
// experts think that this attempt has failed (Heya Macbeth!)).
//
// The driver mainly implements the setting/querying of the (e)uids -- it is
// task of the mudlib to give out the right (e)uid to the right object, and
// to check them where necessary.
//
// If the driver is set to use 'strict euids', the loading and cloning
// of objects requires the initiating object to have a non-zero euid.
//
// The main use for (e)uids is for determination of file access rights, but
// you can of course use the (e)uids for other identification purposes as well.
//===========================================================================

string get_bb_uid() {
  // XXX don't think we want to use process_string (security)
  return 0;
}

int valid_seteuid(object obj, string neweuid) {
  // FUTURE implement uids
  return 1;
}

mixed valid_read(string path, string euid, string fun, object caller) {
  // FUTURE implement security
  // if caller is command
  //   check THISP's command policy, levels:
  //     grant command my access always
  //     grant command my access if unforced
  //     grant command my access never (default)
  // else if caller is avatar
  //   check access file of user's home dir
  //   .access file:
  //     Pattern .
  //
  //   TODO add chroot type thing to narrow access
  //   TODO add group access and membership support
  // else
  //   check dirname of objectname for access file
  return 1;
}

mixed valid_write(string path, string euid, string fun, object caller) {
  // FUTURE implement security
  return 1;
}


//===========================================================================
//  ed() Support
//
//===========================================================================

string make_path_absolute(string str) {
  // TODO implement path expansion
  return str;
}

int save_ed_setup(object who, int code) {
  // TODO implement ed setup
  return 0;
}

int retrieve_ed_setup(object who) {
  // TODO implement ed setup
  return 0;
}

string get_ed_buffer_save_file_name(string file) {
  // FUTURE implement ed buffer save file
  return "/ed_buffer_save_file";
}

/****************************************************************************/
