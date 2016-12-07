/**
 * Utility library for tracking connections.
 *
 * @author devo@eotl
 * @alias ConnectionLib
 */

#include <capabilities.h>
#include <command_giver.h>

inherit ObjectLib;

#define DEFAULT_SCREEN_WIDTH   80
#define DEFAULT_SCREEN_LENGTH  25
#define DEFAULT_PROMPT         "> "

struct ConnectionInfo {
  string id;
  string terminal;
  int terminal_width, terminal_height;
  int *ttyloc;
  int connect_time;
  int disconnect_time;
};

struct ConnectionState {
  object interactive;
  int exec_time;
  mapping negotiation_pending;
  int naws_last;
  struct ConnectionInfo info;
  string session;
};

int connect_session(object who, string session) {
  object logger = LoggerFactory->get_logger(THISO);
  object avatar = SessionTracker->query_avatar(session);
  if (!avatar) {
    logger->debug("no avatar attached to session: %O %O", session, who);
    return 0;
  }
  if (!is_capable(avatar, CAP_AVATAR)) {
    logger->debug("attempting to connect a non-avatar: %O %O", avatar, who);
    return 0;
  }
  if (!exec(avatar, who)) {
    logger->debug("exec failed: %O %O", avatar, who);
    return 0;
  }
  string connection = ConnectionTracker->query_connection(who);
  if (!ConnectionTracker->set_interactive(connection, avatar)) {
    logger->debug("failed to switch connection: %O %O", connection, avatar);
    return 0;
  }
  if (!SessionTracker->connect_session(session, connection)) {
    logger->debug("failed to connect session: %O %O", session, connection);
    return 0;
  }
  return 1;
}

/**
 * Returns the user's screen width. Currently hard-coded to 80 characters.
 * @return the user's screen width
 */
public int query_screen_width(object ob) {
  if (!ob) {
    ob = THISO;
  }
  struct ConnectionInfo info = 
    ConnectionTracker->query_connection_info(
      ConnectionTracker->query_connection_id(ob)
    );
  int result = info->terminal_width;
  if (result <= 0) {
    result = DEFAULT_SCREEN_WIDTH;
  }
  return result;
}

/**
 * Returns the user's screen length. Currently hard-coded to 25 lines.
 * @return the user's screen length
 */
public int query_screen_length(object ob) {
  if (!ob) {
    ob = THISO;
  }
  struct ConnectionInfo info = 
    ConnectionTracker->query_connection_info(
      ConnectionTracker->query_connection_id(ob)
    );
  int result = info->terminal_width;
  if (result <= 0) {
    result = DEFAULT_SCREEN_LENGTH;
  }
  return result;
}

/**
 * Return the user's terminal type, as detected by telnet negotation.
 * 
 * @return a string designating the terminal type
 */
public string query_terminal_type(object ob) {
  if (!ob) {
    ob = THISO;
  }
  struct ConnectionInfo info = 
    ConnectionTracker->query_connection_info(
      ConnectionTracker->query_connection_id(ob)
    );
  if (info) {
    return info->terminal;
  }
  return 0;
}

/**
 * Restore the default prompt.
 */
public void restore_prompt(object ob) {
  if (!ob) {
    ob = THISO;
  }
  if (is_capable(ob, CAP_SHELL)) {
    set_prompt(lambda(0,
      ({ #'+,
        ({ #'call_other, ob, "query_context" }),
        DEFAULT_PROMPT
      })
    ), ob);
  } else {
    set_prompt(DEFAULT_PROMPT, ob);
  }
}
