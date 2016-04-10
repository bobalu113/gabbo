/**
 * A login object to handle incoming connections.
 *
 * @author devo@eotl
 * @alias LoginObject
 */
#include <ansi.h>
#include <sys/input_to.h>

inherit CommandGiverMixin;
inherit SoulMixin;
inherit AvatarMixin;
inherit SensorMixin;

private variables private functions inherit MessageLib;
private variables private functions inherit ObjectLib;

#define LOCALHOST              "127.0.0.1"
#define WELCOME_FILE           _EtcDir "/issue"
#define WELCOME_TOPIC          "system.login"
#define TERMINAL_MAX_TRIES     30
#define TIMEOUT_SECS           (10 * 60)
#define DEFAULT_TERMINAL_TYPE  "vt100"
#define InsecureWarning        "Warning: You are on an insecure connection. " \
                               "Act accordingly.\n"

int logon();
varargs void get_terminal_type(closure callback, int retry);
static void welcome(string terminal, int is_default);
static void timeout();
static void abort();
void heart_beat();

/**
 * Invoked by the master object when a new connection is established.
 *
 * @return 1 for success, 0 for failure
 */
int logon() {
  object logger = LoggerFactory->getLogger(THISO);
  if (caller_stack_depth() > 0) {
    return 0;
  }
  ConnectionTracker->new_connection(THISO);
  ConnectionTracker->telnet_get_terminal(THISO);
  input_to("dummy");
  get_terminal_type(#'welcome); //'
  set_heart_beat(1);
  return 1;
}

/**
 * Retry loop to wait for terminal type telnet negotation. If term type isn't
 * set before MAX_RETRIES, a default terminal type will be used.
 * 
 * @param callback upon detection, run this callback with terminal type
 * @param retry    designates which how many retries have been run
 */
varargs void get_terminal_type(closure callback, int retry) {
  string term = query_terminal_type();
  if (term) {
    funcall(callback, term);
  } else {
    if (retry >= TERMINAL_MAX_TRIES) {      
      funcall(callback, DEFAULT_TERMINAL_TYPE, 1);
    } else {
      call_out(#'get_terminal_type, 1, callback, retry + 1); //'
    }
  }
  return;
}

/**
 * Display the welcome screen.
 * 
 * @param terminal 
 * @param is_default  [description]
 */
static void welcome(string terminal, int is_default) {
  object logger = LoggerFactory->getLogger(THISO);
  int secure = 0;
  string welcome = CLEAR_SCREEN;
  if (is_default) {
    welcome += "Unable to detect terminal type. Using default.\n";
  }
  welcome += read_file(WELCOME_FILE);
  if (query_ip_number(THISO) != LOCALHOST) {
    welcome += InsecureWarning;
  } else {
    secure = 1;
  }
  if (welcome) {
    system_msg(THISO, 
               WELCOME_TOPIC, 
               ([ "message" : welcome,
                  "insecure" : !secure,
                  "defaultTermType" : is_default
               ]));
  } else {
    logger->warn("unable to read welcome file");
  }
  remove_input_to(THISO);
  restore_prompt();
  send_prompt(THISO);
  return;
}

/**
 * Exit login prompt because of idle timeout.
 */
static void timeout() {
  system_msg(THISO,
             WELCOME_TOPIC, 
             ([ "message" : "Timeout exceeded, disconnecting...\n",
                "abort" : 1 ]));
  abort();
}

/**
 * Abort login.
 */
static void abort() {
  destruct(THISO);
}

/**
 * Called every heart beat interval to check for idle timeout.
 */
void heart_beat() {
  if (query_idle(THISO) > TIMEOUT_SECS) {
    timeout();
    set_heart_beat(0);
  }
  return;
}
