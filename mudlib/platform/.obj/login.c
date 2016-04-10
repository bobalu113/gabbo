/**
 * A login object to handle incoming connections.
 *
 * @author devo@eotl
 * @alias LoginObject
 */
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
#define CLEAR_SCREEN           1
#define InsecureWarning        "Warning: You are on an insecure connection. " \
                               "Act accordingly.\n"
#define DefaultTermWarning     "Unable to detect terminal type. " \
                               "Using default.\n"
#define TimeoutMessage         "Timeout exceeded, disconnecting...\n"

int logon();
void suppress_prompt();
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
  input_to("suppress_prompt"); // suppress prompt until welcome screen is shown
  get_terminal_type(#'welcome); //'
  set_heart_beat(1);
  return 1;
}

void suppress_prompt() {
  remove_input_to(THISO);
  input_to("suppress_prompt");
  return;
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
static void welcome(string terminal, mixed is_default) {
  object logger = LoggerFactory->getLogger(THISO);

  if (is_default && !stringp(is_default)) {
    is_default = DefaultTermWarning;
  }
  string insecure = 0;
  if (query_ip_number(THISO) != LOCALHOST) {
    insecure = InsecureWarning;
  }
  string welcome = read_file(WELCOME_FILE);
  if (!welcome) {
    logger->warn("unable to read welcome file");
  }

  // display welcome screen
  system_msg(THISO, 
             WELCOME_TOPIC, 
             ([ "welcome" : welcome,
                "insecure" : insecure,
                "defaultTerm" : is_default,
                "clearScreen" : CLEAR_SCREEN
             ]));


  // restore prompt
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
             ([ "abort" : TimeoutMessage ]));
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
